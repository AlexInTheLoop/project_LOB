#ifndef PORTFOLIO_H
#define PORTFOLIO_H

#include <string>
#include <map>
#include <vector>

#include "OrderBookManager.h"

struct Trade {
    std::string dateTime;
    std::string stock;
    std::string tradeType;
    double quantity;
    double price;
    double totalAmount;
};

struct Holding{
    double quantity =0.0;
    double averagePrice =0.0;
};

struct PnLRecord{
    std::string dateTime;
    std::string stock;
    double realizedPnL;
    double quantity =0.0;
};

class Portfolio {
public: 
    Portfolio();

    void updateBuy(const std::string &stock, double quantity, double price, const std::string &dateTime);

    void updateSell(const std::string & stock, double quantity, double price, const std::string &dateTime);

    void printHoldings() const;

    void printGlobalPnL() const;

    void logTradesToCSV(const std::string &filename) const;

    void logPnLHistoryToCSV(const std::string &filename) const;

    void printAssetPerformance(const std::map<std::string, OrderBookStatistics>& marketStats) const;

private:

    std::map<std::string, Holding> holdings;
    std::vector<Trade> tradeHistory;
    double globalPnL;
    std::vector<PnLRecord> pnlHistory;
};

#endif