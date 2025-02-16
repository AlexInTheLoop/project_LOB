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

struct Order {
    int id;
    std::string asset;
    std::string timestamp;
    std::string type;
    bool isShortSell;
    double price;
    double quantity;
    double totalAmount;
    std::chrono::system_clock::time_point dateTime;
};

struct OrderBookEntry {
    int id;
    double price;
    double quantity;
    std::chrono::system_clock::time_point timestamp;
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
    std::string csvPath;
    std::vector<Order> orders;
    std::map<std::string, std::map<double, OrderBookEntry, std::greater<>>> bidBooks;
    std::map<std::string, std::map<double, OrderBookEntry>> askBooks;
    std::map<std::string, OrderBookStatistics> statistics;

    std::chrono::system_clock::time_point parseTimestamp(const std::string& timestamp);
    void updateStatistics(const std::string& asset);

public:
    OrderBookManager(const std::string& path);
    void loadOrders();
    void processOrders();
    void displayOrderBooks();
    void displayOrderBook(const std::string& asset);
    void saveOrderBooks(const std::string& outputPath);
    void processNewOrder(const Order& order);
    const std::map<std::string, OrderBookStatistics>& getStatistics() const { return statistics; }
};

#endif
