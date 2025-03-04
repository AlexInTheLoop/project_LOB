#include "OrderBookSimulator.h"

using namespace std;

int TIME_INTERVAL{20};

OrderBookSimulator::OrderBookSimulator(OrderBookManager& ob): orderBook(ob) {
    const auto& stats{orderBook.getStatistics()};
    for (const auto& [asset, _] : stats) {
        assets.push_back(asset);
    }
    initializeGenerators();
}

void OrderBookSimulator::initializeGenerators() {
    random_device rd;
    for (const auto& asset : assets) {
        generators[asset] = mt19937(rd());
        volumeDists[asset] = uniform_real_distribution<>(0.1, 1000.0);
        marketLimitDists[asset] = bernoulli_distribution(0.5);
        buySellDists[asset] = bernoulli_distribution(0.5);
    }
}

Order OrderBookSimulator::generateOrder(const string& asset, double minPrice, 
                                      double maxPrice, double midPrice) {
    Order order;
    order.asset = asset;

    bool isBuyOrder{buySellDists[asset](generators[asset])};
    order.type = isBuyOrder ? "BUY" : "SELL";
    order.isShortSell = false;

    order.quantity = volumeDists[asset](generators[asset]);

    bool isMarketOrder{marketLimitDists[asset](generators[asset])};
    string orderCategory = isMarketOrder ? "MARKET" : "LIMIT";

    if (isMarketOrder) {
        order.price = isBuyOrder ? maxPrice : minPrice;
        order.quantity = order.quantity * 10;
    } else {
        normal_distribution<> normalDist(midPrice, 3);
        order.price = normalDist(generators[asset]);

        if (isBuyOrder && order.price >= maxPrice) {
            order.price = maxPrice;
            orderCategory = "MARKET";
        } else if (!isBuyOrder && order.price <= minPrice) {
            order.price = minPrice;
            orderCategory = "MARKET";
        }
    }

    order.totalAmount = order.price * order.quantity;

    auto now{chrono::system_clock::now()};
    auto now_time_t {chrono::system_clock::to_time_t(now)};
    stringstream ss;
    ss << put_time(localtime(&now_time_t), "%Y-%m-%d %H:%M:%S");
    order.timestamp = ss.str();
    order.dateTime = now;

    cout << "\n==== New order for " << asset << " ====" << endl;
    cout << "Timestamp: " << order.timestamp << endl;
    cout << "Type: " << order.type << " (" << orderCategory << ")" << endl;
    cout << "Price: " << fixed << setprecision(3) << order.price << endl;
    cout << "Quantity: " << order.quantity << endl;
    cout << "Total Amount: " << order.totalAmount << endl;
    cout << "================================" << endl;

    return order;
}

void OrderBookSimulator::simulateRealtime(int durationSeconds) {
    auto startTime{chrono::steady_clock::now()};
    bool running{true};
    cout << "Simulation starting...\n" << endl;
    cout << "Available assets: ";
    for (const auto& asset : assets) {
        cout << asset << " ";
    }

    while (running) {
        const auto& stats{orderBook.getStatistics()};
        
        for (const auto& asset : assets) {
            const auto& assetStats{stats.at(asset)};
            
            double minPrice{assetStats.bidPrice};
            double maxPrice{assetStats.askPrice};
            double midPrice{assetStats.midPrice};

            if (minPrice <= 0 || maxPrice <= 0 || midPrice <= 0) {
                continue;
            }

            Order newOrder{generateOrder(asset, minPrice, maxPrice, midPrice)};
            orderBook.processNewOrder(newOrder);
            orderBook.displayOrderBook(asset);
        }
        this_thread::sleep_for(chrono::seconds(TIME_INTERVAL));
        if (durationSeconds > 0) {
            auto currentTime{chrono::steady_clock::now()};
            auto elapsed{chrono::duration_cast<chrono::seconds>(currentTime - startTime).count()};
            if (elapsed >= durationSeconds) {
                running = false;
            }
        }
    }
}