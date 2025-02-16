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
        cerr << "Error: The amount of assets requested is higher than the number of available assets" << endl;
        return;
    }

    if (nbOrders.size() != static_cast<size_t>(nbAssets) || 
        prices.size() != static_cast<size_t>(nbAssets) ||
        (shortRatios.size() != 1 && shortRatios.size() != static_cast<size_t>(nbAssets))) {
        cerr << "Error: Vectors must have the same dimension" << endl;
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
        cerr << "Error: File access denied" << endl;
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
    cout << "Orders generated in the file: " << outputFilename << endl;
}


vector<Order> generateOrdersAndReturn(
    int nbAssets, 
    const vector<int>& nbOrders,
    const vector<double>& prices, 
    const vector<double>& shortRatios,
    const string& outputFilename
) {
    vector<Order> generatedOrders;

    if (nbAssets > static_cast<int>(ASSETS.size())) {
        cerr << "Error: The amount of assets requested is higher than the number of available assets" << endl;
        return generatedOrders; // empty
    }

    if (nbOrders.size() != static_cast<size_t>(nbAssets) || 
        prices.size()   != static_cast<size_t>(nbAssets) ||
        (shortRatios.size() != 1 && shortRatios.size() != static_cast<size_t>(nbAssets))) {
        cerr << "Error: Vectors must have the same dimension" << endl;
        return generatedOrders; // empty
    }

    set<int> selectedAssetsIndices;
    while (selectedAssetsIndices.size() < static_cast<size_t>(nbAssets)) {
        selectedAssetsIndices.insert(static_cast<int>(generateRandomUniform(0, ASSETS.size())));
    }

    vector<string> selectedAssets;
    for (int idx : selectedAssetsIndices) {
        selectedAssets.push_back(ASSETS[idx]);
    }

    vector<double> adjustedShortRatios = (shortRatios.size() == 1)
        ? vector<double>(nbAssets, shortRatios[0])
        : shortRatios;

    ofstream file(outputFilename);
    if (!file.is_open()) {
        cerr << "Error: File access denied" << endl;
        return generatedOrders; // empty
    }

    file << "ID,Asset,Timestamp,Type,Is Short Sell,Price,Quantity,Total Amount\n";

    for (size_t i = 0; i < selectedAssets.size(); ++i) {
        const string& asset = selectedAssets[i];
        double meanPrice  = prices[i];
        double shortRatio = adjustedShortRatios[i];
        int ordersForAsset = nbOrders[i];

        int totalSellOrders  = static_cast<int>(round(ordersForAsset / 2.0));
        int shortSellOrders  = static_cast<int>(round(totalSellOrders * shortRatio));
        int totalBuyOrders   = ordersForAsset - totalSellOrders;

        // Generate BUY orders
        for (int j = 0; j < totalBuyOrders; ++j) {
            double price = roundToTickSize(generateRandomNormal(meanPrice, 1.0), 0.1);
            double quantity = generateRandomUniform(0.1, 1000.0);
            double totalAmount = price * quantity;
            string timestamp = generateRandomTimestamp();
            int orderID = static_cast<int>(round(generateRandomUniform(1, 300)));

            // Write to CSV
            file << orderID << "," << asset << "," << timestamp << ","
                 << "BUY,False," << price << "," << quantity << ","
                 << setprecision(15) << totalAmount << "\n";

            Order newOrder;
            newOrder.id          = orderID;
            newOrder.asset       = asset;
            newOrder.timestamp   = timestamp;
            newOrder.type        = "BUY";
            newOrder.isShortSell = false;
            newOrder.price       = price;
            newOrder.quantity    = quantity;
            newOrder.totalAmount = totalAmount;

            tm tmStruct = {};
            istringstream ss(timestamp);
            ss >> get_time(&tmStruct, "%Y-%m-%d %H:%M:%S");
            newOrder.dateTime = chrono::system_clock::from_time_t(mktime(&tmStruct));

            generatedOrders.push_back(newOrder);
        }

        // Generate SELL orders
        for (int j = 0; j < totalSellOrders; ++j) {
            double price = roundToTickSize(generateRandomNormal(meanPrice, 1.0), 0.1);
            double quantity = generateRandomUniform(0.1, 1000.0);
            double totalAmount = price * quantity;
            string timestamp = generateRandomTimestamp();
            bool isShortSell = (j < shortSellOrders);
            int orderID = static_cast<int>(round(generateRandomUniform(1, 300)));

            file << orderID << "," << asset << "," << timestamp << ","
                 << "SELL," << (isShortSell ? "True" : "False") << ","
                 << price << "," << quantity << ","
                 << setprecision(15) << totalAmount << "\n";

            Order newOrder;
            newOrder.id          = orderID;
            newOrder.asset       = asset;
            newOrder.timestamp   = timestamp;
            newOrder.type        = "SELL";
            newOrder.isShortSell = isShortSell;
            newOrder.price       = price;
            newOrder.quantity    = quantity;
            newOrder.totalAmount = totalAmount;

            tm tmStruct = {};
            istringstream ss(timestamp);
            ss >> get_time(&tmStruct, "%Y-%m-%d %H:%M:%S");
            newOrder.dateTime = chrono::system_clock::from_time_t(mktime(&tmStruct));

            generatedOrders.push_back(newOrder);
        }
    }

    file.close();
    cout << "Orders generated in the file: " << outputFilename << endl;
    return generatedOrders;
}
