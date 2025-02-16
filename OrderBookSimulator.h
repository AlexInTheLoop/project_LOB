#ifndef ORDER_BOOK_SIMULATOR_H
#define ORDER_BOOK_SIMULATOR_H

#include "OrderBookManager.h"
#include <random>
#include <chrono>
#include <thread>
#include <iomanip>
#include <sstream>
#include <map>
#include <vector>
#include <string>

class OrderBookSimulator {
private:
    OrderBookManager& orderBook;
    std::vector<std::string> assets;
    std::map<std::string, std::mt19937> generators;
    std::map<std::string, std::uniform_real_distribution<>> volumeDists;
    std::map<std::string, std::bernoulli_distribution> marketLimitDists;
    std::map<std::string, std::bernoulli_distribution> buySellDists;

    Order generateOrder(const std::string& asset, double minPrice, double maxPrice, 
                       double midPrice);
    void initializeGenerators();

public:
    OrderBookSimulator(OrderBookManager& ob);
    void simulateRealtime(int durationSeconds = -1);
};

#endif