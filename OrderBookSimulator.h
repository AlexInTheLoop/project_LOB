#ifndef ORDER_BOOK_SIMULATOR_H
#define ORDER_BOOK_SIMULATOR_H

#include "OrderBookManager.h"
#include <random>
#include <chrono>
#include <thread>
#include <iomanip>
#include <sstream>
#include <map>

using namespace std;

class OrderBookSimulator {
private:
    OrderBookManager& orderBook;
    vector<string> assets;
    map<string, mt19937> generators;
    map<string, uniform_real_distribution<>> volumeDists;
    map<string, bernoulli_distribution> marketLimitDists;
    map<string, bernoulli_distribution> buySellDists;
    map<string, int> currentOrderIds;

    Order generateOrder(const string& asset, double minPrice, double maxPrice, 
                       double midPrice, double spread);
    void initializeGenerators();

public:
    OrderBookSimulator(OrderBookManager& ob);
    void simulateRealtime(int durationSeconds = -1);
};

#endif