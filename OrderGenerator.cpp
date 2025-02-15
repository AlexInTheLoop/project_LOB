#include "OrderGenerator.h"
#include "OrderBookManager.h"

using namespace std;

const vector<string> ASSETS {"AAPL", "TSLA", "GOOG", "MSFT", "AMZN", "META", "NFLX", "NVDA"};

double generateRandomNormal(double mean, double variance) {
    static random_device rd;
    static mt19937 gen(rd());
    normal_distribution<> ndis(mean, sqrt(variance));
    return ndis(gen);
}

double generateRandomUniform(double lower, double upper) {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_real_distribution<> udis(lower, upper);
    return udis(gen);
}

double roundToTickSize(double value, double tickSize) {
    return round(value / tickSize) * tickSize;
}

string generateRandomTimestamp() {
    int day = static_cast<int>(generateRandomUniform(1, 29));
    int hour = static_cast<int>(generateRandomUniform(0, 24));
    int minute = static_cast<int>(generateRandomUniform(0, 60));
    int second = static_cast<int>(generateRandomUniform(0, 60));

    ostringstream timestamp;
    timestamp << "2025-02-";
    if (day < 10) timestamp << "0";
    timestamp << day << " ";
    if (hour < 10) timestamp << "0";
    timestamp << hour << ":";
    if (minute < 10) timestamp << "0";
    timestamp << minute << ":";
    if (second < 10) timestamp << "0";
    timestamp << second;

    return timestamp.str();
}

void generateOrders(int nbAssets, const vector<int>& nbOrders,
                   const vector<double>& prices, const vector<double>& shortRatios,
                   const string& outputFilename) {
    if (nbAssets > static_cast<int>(ASSETS.size())) {
        cerr << "Erreur : Le nombre d'actifs voulu est superieur au nombre d'actifs disponibles" << endl;
        return;
    }

    if (nbOrders.size() != static_cast<size_t>(nbAssets) || 
        prices.size() != static_cast<size_t>(nbAssets) ||
        (shortRatios.size() != 1 && shortRatios.size() != static_cast<size_t>(nbAssets))) {
        cerr << "Erreur : Les vecteurs en arguments n'ont pas la meme taille." << endl;
        return;
    }

    set<int> selectedAssetsIndices;
    while (selectedAssetsIndices.size() < static_cast<size_t>(nbAssets)) {
        selectedAssetsIndices.insert(static_cast<int>(generateRandomUniform(0, ASSETS.size())));
    }

    vector<string> selectedAssets;
    for (int idx : selectedAssetsIndices) {
        selectedAssets.push_back(ASSETS[idx]);
    }

    vector<double> adjustedShortRatios = (shortRatios.size() == 1) ? 
                                             vector<double>(nbAssets, shortRatios[0]) : shortRatios;

    ofstream file(outputFilename);
    if (!file.is_open()) {
        std::cerr << "Erreur : Le fichier ne peut etre ouvert." << std::endl;
        return;
    }

    file << "ID,Asset,Timestamp,Type,Is Short Sell,Price,Quantity,Total Amount\n";

    for (size_t i = 0; i < selectedAssets.size(); ++i) {
        const string& asset {selectedAssets[i]};
        double meanPrice {prices[i]};
        double shortRatio {adjustedShortRatios[i]};
        int ordersForAsset {nbOrders[i]};

        int totalSellOrders {static_cast<int>(round(ordersForAsset / 2.0))};
        int shortSellOrders {static_cast<int>(round(totalSellOrders * shortRatio))};

        int totalBuyOrders {ordersForAsset - totalSellOrders};
        for (int j = 0; j < totalBuyOrders; ++j) {
            double price {roundToTickSize(generateRandomNormal(meanPrice, 1.0), 0.1)};
            double quantity {generateRandomUniform(0.1, 1000.0)};
            double totalAmount {price * quantity};
            string timestamp {generateRandomTimestamp()};
            int orderID {static_cast<int>(round(generateRandomUniform(1, 300)))};

            file << orderID << "," << asset << "," << timestamp << ","
                 << "BUY,False," << price << "," << quantity << "," 
                 << setprecision(15) << totalAmount << "\n";

        }

        for (int j = 0; j < totalSellOrders; ++j) {
            double price {roundToTickSize(generateRandomNormal(meanPrice, 1.0), 0.1)};
            double quantity {generateRandomUniform(0.1, 1000.0)};
            double totalAmount {price * quantity};
            string timestamp {generateRandomTimestamp()};
            bool isShortSell {(j < shortSellOrders)};
            int orderID {static_cast<int>(round(generateRandomUniform(1, 300)))};

            file << orderID << "," << asset << "," << timestamp << ","
                 << "SELL," << (isShortSell ? "True" : "False") << "," << price << "," 
                 << quantity << "," << setprecision(15) << totalAmount << "\n";

        }
    }

    file.close();
    cout << "Ordres produits dans le fichier : " << outputFilename << endl;
}


std::vector<Order> generateOrdersAndReturn(
    int nbAssets, 
    const std::vector<int>& nbOrders,
    const std::vector<double>& prices, 
    const std::vector<double>& shortRatios,
    const std::string& outputFilename
) {
    // We'll collect and return these
    std::vector<Order> generatedOrders;

    if (nbAssets > static_cast<int>(ASSETS.size())) {
        std::cerr << "Erreur : Le nombre d'actifs voulu est superieur au nombre d'actifs disponibles" << std::endl;
        return generatedOrders; // empty
    }

    if (nbOrders.size() != static_cast<size_t>(nbAssets) || 
        prices.size()   != static_cast<size_t>(nbAssets) ||
        (shortRatios.size() != 1 && shortRatios.size() != static_cast<size_t>(nbAssets))) {
        std::cerr << "Erreur : Les vecteurs en arguments n'ont pas la meme taille." << std::endl;
        return generatedOrders; // empty
    }

    std::set<int> selectedAssetsIndices;
    while (selectedAssetsIndices.size() < static_cast<size_t>(nbAssets)) {
        selectedAssetsIndices.insert(static_cast<int>(generateRandomUniform(0, ASSETS.size())));
    }

    std::vector<std::string> selectedAssets;
    for (int idx : selectedAssetsIndices) {
        selectedAssets.push_back(ASSETS[idx]);
    }

    std::vector<double> adjustedShortRatios = (shortRatios.size() == 1)
        ? std::vector<double>(nbAssets, shortRatios[0])
        : shortRatios;

    std::ofstream file(outputFilename);
    if (!file.is_open()) {
        std::cerr << "Erreur : Le fichier ne peut etre ouvert." << std::endl;
        return generatedOrders; // empty
    }

    // CSV header
    file << "ID,Asset,Timestamp,Type,Is Short Sell,Price,Quantity,Total Amount\n";

    for (size_t i = 0; i < selectedAssets.size(); ++i) {
        const std::string& asset = selectedAssets[i];
        double meanPrice  = prices[i];
        double shortRatio = adjustedShortRatios[i];
        int ordersForAsset = nbOrders[i];

        int totalSellOrders  = static_cast<int>(std::round(ordersForAsset / 2.0));
        int shortSellOrders  = static_cast<int>(std::round(totalSellOrders * shortRatio));
        int totalBuyOrders   = ordersForAsset - totalSellOrders;

        // Generate BUY orders
        for (int j = 0; j < totalBuyOrders; ++j) {
            double price = roundToTickSize(generateRandomNormal(meanPrice, 1.0), 0.1);
            double quantity = generateRandomUniform(0.1, 1000.0);
            double totalAmount = price * quantity;
            std::string timestamp = generateRandomTimestamp();
            int orderID = static_cast<int>(std::round(generateRandomUniform(1, 300)));

            // Write to CSV
            file << orderID << "," << asset << "," << timestamp << ","
                 << "BUY,False," << price << "," << quantity << ","
                 << std::setprecision(15) << totalAmount << "\n";

            // Also store in the vector
            Order newOrder;
            newOrder.id          = orderID;
            newOrder.asset       = asset;
            newOrder.timestamp   = timestamp;
            newOrder.type        = "BUY";
            newOrder.isShortSell = false;
            newOrder.price       = price;
            newOrder.quantity    = quantity;
            newOrder.totalAmount = totalAmount;

            // We'll parse timestamp into dateTime as well (if you want).
            // Or you could leave it for the Manager's loadOrders() to parse.
            // For completeness, let's do it:
            tm tmStruct = {};
            std::istringstream ss(timestamp);
            ss >> std::get_time(&tmStruct, "%Y-%m-%d %H:%M:%S");
            newOrder.dateTime = std::chrono::system_clock::from_time_t(std::mktime(&tmStruct));

            generatedOrders.push_back(newOrder);
        }

        // Generate SELL orders
        for (int j = 0; j < totalSellOrders; ++j) {
            double price = roundToTickSize(generateRandomNormal(meanPrice, 1.0), 0.1);
            double quantity = generateRandomUniform(0.1, 1000.0);
            double totalAmount = price * quantity;
            std::string timestamp = generateRandomTimestamp();
            bool isShortSell = (j < shortSellOrders);
            int orderID = static_cast<int>(std::round(generateRandomUniform(1, 300)));

            // Write to CSV
            file << orderID << "," << asset << "," << timestamp << ","
                 << "SELL," << (isShortSell ? "True" : "False") << ","
                 << price << "," << quantity << ","
                 << std::setprecision(15) << totalAmount << "\n";

            // Also store in vector
            Order newOrder;
            newOrder.id          = orderID;
            newOrder.asset       = asset;
            newOrder.timestamp   = timestamp;
            newOrder.type        = "SELL";
            newOrder.isShortSell = isShortSell;
            newOrder.price       = price;
            newOrder.quantity    = quantity;
            newOrder.totalAmount = totalAmount;

            // parse into dateTime
            tm tmStruct = {};
            std::istringstream ss(timestamp);
            ss >> std::get_time(&tmStruct, "%Y-%m-%d %H:%M:%S");
            newOrder.dateTime = std::chrono::system_clock::from_time_t(std::mktime(&tmStruct));

            generatedOrders.push_back(newOrder);
        }
    }

    file.close();
    std::cout << "Ordres produits dans le fichier : " << outputFilename << std::endl;

    // Return the in-memory vector of all generated orders
    return generatedOrders;
}
