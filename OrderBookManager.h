#ifndef ORDER_BOOK_MANAGER_H
#define ORDER_BOOK_MANAGER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <iomanip>
#include <chrono>

using namespace std;

struct Order {
    int id;
    string asset;
    string timestamp;
    string type;
    bool isShortSell;
    double price;
    double quantity;
    double totalAmount;
    chrono::system_clock::time_point dateTime;
};

struct OrderBookEntry {
    int id;
    double price;
    double quantity;
    chrono::system_clock::time_point timestamp;
};

struct OrderBookStatistics {
    double averageExecutedPrice{0.0};
    double totalTradedQuantity{0.0};
    double totalTradedAmount{0.0};
    double bidPrice{0.0};
    double askPrice{0.0};
    double midPrice{0.0};
    double bidAskSpread{0.0};
    int bidDepth{0};
    int askDepth{0};
    double totalBidAmount{0.0};
    double totalAskAmount{0.0};
};

class OrderBookManager {
private:
    string csvPath;
    vector<Order> orders;
    map<string, map<double, OrderBookEntry, greater<>>> bidBooks;
    map<string, map<double, OrderBookEntry>> askBooks;
    map<string, OrderBookStatistics> statistics;

    chrono::system_clock::time_point parseTimestamp(const string& timestamp);
    void updateStatistics(const string& asset);

public:
    OrderBookManager(const string& path);
    void loadOrders();
    void processOrders();
    void displayOrderBooks();
    void displayOrderBook(const string& asset);
    void saveOrderBooks(const string& outputPath);
    void processNewOrder(const Order& order);
    const map<string, OrderBookStatistics>& getStatistics() const { return statistics; }
};

#endif