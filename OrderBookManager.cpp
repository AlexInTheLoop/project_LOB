#include "OrderBookManager.h"

using namespace std;

OrderBookManager::OrderBookManager(const string& path) : csvPath(path) {}

chrono::system_clock::time_point OrderBookManager::parseTimestamp(const string& timestamp) {
    tm tm {};
    istringstream ss(timestamp);
    ss >> get_time(&tm, "%Y-%m-%d %H:%M:%S");
    return chrono::system_clock::from_time_t(mktime(&tm));
}

void OrderBookManager::loadOrders() {
    ifstream file(csvPath);
    string line;
    
    getline(file, line);
    
    while (getline(file, line)) {
        stringstream ss(line);
        string field;
        Order order;
        
        getline(ss, field, ',');
        order.id = stoi(field);
        
        getline(ss, order.asset, ',');
        getline(ss, order.timestamp, ',');
        getline(ss, order.type, ',');
        
        getline(ss, field, ',');
        order.isShortSell = (field == "True");
        
        getline(ss, field, ',');
        order.price = stod(field);
        
        getline(ss, field, ',');
        order.quantity = std::stod(field);
        
        getline(ss, field, ',');
        order.totalAmount = std::stod(field);
        
        order.dateTime = parseTimestamp(order.timestamp);
        orders.push_back(order);
    }
}

void OrderBookManager::processOrders() {
    for (const auto& order : orders) {
        if (order.type == "BUY") {
            OrderBookEntry entry{order.id, order.price, order.quantity, order.dateTime};
            bidBooks[order.asset].insert({order.price, entry});
        } else {
            OrderBookEntry entry{order.id, order.price, order.quantity, order.dateTime};
            askBooks[order.asset].insert({order.price, entry});
        }
    }

    for (const auto& asset : bidBooks) {
        auto& bidBook{bidBooks[asset.first]};
        auto& askBook{askBooks[asset.first]};
        auto& stats{statistics[asset.first]};

        while (!bidBook.empty() && !askBook.empty()) {
            auto bid{bidBook.begin()};
            auto ask{askBook.begin()};

            if (bid->first < ask->first) break;

            double execQuantity{min(bid->second.quantity, ask->second.quantity)};
            double execPrice{ask->first};

            stats.totalTradedQuantity += execQuantity;
            stats.totalTradedAmount += execQuantity * execPrice;
            
            if (bid->second.quantity > execQuantity) {
                OrderBookEntry newEntry{bid->second};
                newEntry.quantity -= execQuantity;
                bidBook.erase(bid);
                bidBook.insert({newEntry.price, newEntry});
            } else {
                bidBook.erase(bid);
            }

            if (ask->second.quantity > execQuantity) {
                OrderBookEntry newEntry{ask->second};
                newEntry.quantity -= execQuantity;
                askBook.erase(ask);
                askBook.insert({newEntry.price, newEntry});
            } else {
                askBook.erase(ask);
            }
        }

        updateStatistics(asset.first);
    }
}

void OrderBookManager::displayOrderBooks() {
    for (const auto& asset : bidBooks) {
        displayOrderBook(asset.first);
    }
}

void OrderBookManager::saveOrderBooks(const std::string& outputPath) {
    system(("mkdir " + outputPath + " 2> nul").c_str());

    for (const auto& asset : bidBooks) {
        string filename{outputPath + "/" + asset.first + "_orderbook.csv"};
        ofstream file(filename, ios::out | ios::trunc);
        
        if (!file.is_open()) {
            cerr << "Erreur : Impossible d'ouvrir le fichier " << filename << "\n";
            continue;
        }

        file << fixed << setprecision(2);
        file << "BID VOLUME,PRICE,ASK VOLUME\n";

        set<double, greater<double>> allPrices;
        for (const auto& bid : bidBooks[asset.first]) allPrices.insert(bid.first);
        for (const auto& ask : askBooks[asset.first]) allPrices.insert(ask.first);

        for (double price : allPrices) {
            auto bid{bidBooks[asset.first].find(price)};
            auto ask{askBooks[asset.first].find(price)};
            
            if (bid != bidBooks[asset.first].end()) {
                file << bid->second.quantity;
            }
            file << ",";
            
            file << price << ",";
            
            if (ask != askBooks[asset.first].end()) {
                file << ask->second.quantity;
            }
            file << "\n";
        }

        file.close();
    }
}

void OrderBookManager::updateStatistics(const std::string& asset) {
    auto& stats{statistics[asset]};
    auto& bidBook{bidBooks[asset]};
    auto& askBook{askBooks[asset]};

    if (!bidBook.empty()) {
        stats.bidPrice = bidBook.begin()->first;
        stats.bidDepth = bidBook.size();
        stats.totalBidAmount = 0;
        for (const auto& bid : bidBook) {
            stats.totalBidAmount += bid.first * bid.second.quantity;
        }
    } else {
        stats.bidPrice = 0.0;
        stats.bidDepth = 0;
        stats.totalBidAmount = 0.0;
    }

    if (!askBook.empty()) {
        stats.askPrice = askBook.begin()->first;
        stats.askDepth = askBook.size();
        stats.totalAskAmount = 0;
        for (const auto& ask : askBook) {
            stats.totalAskAmount += ask.first * ask.second.quantity;
        }
    } else {
        stats.askPrice = 0.0;
        stats.askDepth = 0;
        stats.totalAskAmount = 0.0;
    }

    if (stats.totalTradedQuantity > 0) {
        stats.averageExecutedPrice = stats.totalTradedAmount / stats.totalTradedQuantity;
    }

    if (stats.bidPrice > 0 && stats.askPrice > 0) {
        stats.midPrice = (stats.bidPrice + stats.askPrice) / 2;
        stats.bidAskSpread = stats.askPrice - stats.bidPrice;
    }
}

void OrderBookManager::processNewOrder(const Order& order) {
    if (order.type == "BUY") {
        OrderBookEntry entry{order.id, order.price, order.quantity, order.dateTime};
        bidBooks[order.asset].insert({order.price, entry});
    } else {
        OrderBookEntry entry{order.id, order.price, order.quantity, order.dateTime};
        askBooks[order.asset].insert({order.price, entry});
    }

    auto& bidBook = bidBooks[order.asset];
    auto& askBook = askBooks[order.asset];
    auto& stats = statistics[order.asset];

    while (!bidBook.empty() && !askBook.empty()) {
        auto bid{bidBook.begin()};
        auto ask{askBook.begin()};

        if (bid->first < ask->first) break;

        double execQuantity{min(bid->second.quantity, ask->second.quantity)};
        double execPrice{ask->first};
        
        stats.totalTradedQuantity += execQuantity;
        stats.totalTradedAmount += execPrice * execQuantity;

        if (bid->second.quantity > execQuantity) {
            OrderBookEntry newEntry{bid->second};
            newEntry.quantity -= execQuantity;
            bidBook.erase(bid);
            bidBook.insert({newEntry.price, newEntry});
        } else {
            bidBook.erase(bid);
        }

        if (ask->second.quantity > execQuantity) {
            OrderBookEntry newEntry{ask->second};
            newEntry.quantity -= execQuantity;
            askBook.erase(ask);
            askBook.insert({newEntry.price, newEntry});
        } else {
            askBook.erase(ask);
        }
    }

    updateStatistics(order.asset);
}

void OrderBookManager::displayOrderBook(const string& asset) {
    auto it = bidBooks.find(asset);
    if (it == bidBooks.end()) {
        std::cout << "Actif inconnu : " << asset << std::endl;
        return;
    }

    cout << "\n" << asset << "\n";
    cout << string(60, '=') << "\n";
    
    cout << setw(15) << left << "BID VOLUME" 
              << setw(15) << right << "PRICE" 
              << setw(15) << right << "ASK VOLUME" << "\n";
    cout << string(60, '-') << "\n";

    set<double, greater<double>> allPrices;
    for (const auto& bid : bidBooks[asset]) allPrices.insert(bid.first);
    for (const auto& ask : askBooks[asset]) allPrices.insert(ask.first);

    for (double price : allPrices) {
        cout << fixed << setprecision(2);
        
        auto bid{bidBooks[asset].find(price)};
        auto ask{askBooks[asset].find(price)};

        if (bid != bidBooks[asset].end()) {
            cout << setw(15) << left << bid->second.quantity;
        } else {
            cout << setw(15) << " ";
        }
        
        cout << setw(15) << right << price;
        
        if (ask != askBooks[asset].end()) {
            std::cout << setw(15) << right << ask->second.quantity;
        }
        std::cout << "\n";
    }

    const auto& stats{statistics[asset]};
    cout << "\nStatistiques " << asset << ":\n";
    cout << fixed << setprecision(2);
    cout << "Prix d'execution moyen : " << stats.averageExecutedPrice << "\n";
    cout << "Volume total echange : " << stats.totalTradedQuantity << "\n";
    cout << "Montant total echange : " << fixed << stats.totalTradedAmount << "\n";
    cout << "Prix Bid : " << stats.bidPrice << "\n";
    cout << "Prix Ask : " << stats.askPrice << "\n";
    cout << "Prix mid : " << stats.midPrice << "\n";
    cout << "Bid-ask spread : " << stats.bidAskSpread << "\n";
    cout << "Profondeur du Bid : " << stats.bidDepth << "\n";
    cout << "Profondeur de l'Ask : " << stats.askDepth << "\n";
    cout << "Montant total a l'achat : " << fixed << stats.totalBidAmount << "\n";
    cout << "Montant total a la vente : " << fixed << stats.totalAskAmount << "\n";
}