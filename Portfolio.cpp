#include "Portfolio.h"
#include "OrderBookManager.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>



using namespace std;

Portfolio::Portfolio() : globalPnL(0.0) {
    vector<string> intialStocks = {"AAPL", "TSLA", "GOOG", "MSFT", "AMZN", "META", "NFLX", "NVDA"};
    for (const auto &stock : intialStocks) {
        holdings[stock] = Holding{0.0, 0.0};
    }
}

void Portfolio::updateBuy(const string &stock, double quantity, double price, const string &dateTime){
    Holding &h = holdings[stock];
    double totalCost = h.quantity*h.averagePrice + quantity*price;
    h.quantity += quantity;
    h.averagePrice = (h.quantity >0) ? totalCost/h.quantity : price;

    Trade trade = {dateTime, stock, "BUY", quantity, price, quantity*price};
    tradeHistory.push_back(trade); 
    cout << "Updated portfolio with buy of " << quantity << " shares of " << stock << " at " << price << endl;
}

void Portfolio::updateSell(const string & stock, double quantity, double price, const string &dateTime){
    if (holdings.find(stock) == holdings.end() || holdings[stock].quantity < quantity){
        cout << "Cannot sell " << quantity << " shares of " << stock << ". Insufficient quantity in portfolio." << endl;
        return;
    }
    Holding &h = holdings[stock];

    double realizedPnL = (price - h.averagePrice)*quantity;
    globalPnL += realizedPnL;

    PnLRecord pnlRecord = {dateTime, stock, realizedPnL, 0.0};
    pnlHistory.push_back(pnlRecord);

    Trade trade = {dateTime, stock, "SELL", quantity, price, quantity*price};
    tradeHistory.push_back(trade);

    cout << "Portfolio updated with sale of " << quantity << " shares of " << stock << " at " << price << endl;
    cout << " Realized PnL: " << realizedPnL << endl;

    h.quantity -= quantity;
    if (h.quantity <= 0){
        holdings.erase(stock);
    }
}

void Portfolio::printHoldings() const{
    cout << "\nPositions actuelles du portefeuille: " << endl;
    if (holdings.empty()){
        cout << "No position in portfolio." << endl;
        return;
    }

    for (const auto &pair : holdings){
        cout << "Stock" << pair.first
            << ",Quantity: " << pair.second.quantity
            << ", Prix moyen $" << pair.second.averagePrice << endl;
        
    }
}

void Portfolio::printGlobalPnL() const{
    cout << "\nGlobal PnL: " << globalPnL << endl;
}


void Portfolio::logTradesToCSV(const std::string &filename) const {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Erreur d'ouverture du fichier CSV des trades du portefeuille." << endl;
        return;
    }
    file << "DateTime,Stock,TradeType,Quantity,Price,TotalAmount\n";
    for (const auto &trade : tradeHistory) {
        file << trade.dateTime << "," << trade.stock << "," << trade.tradeType << ","
             << trade.quantity << "," << trade.price << "," << trade.totalAmount << "\n";
    }
    file.close();
    cout << "Historique des trades du portefeuille enregistré dans " << filename << endl;
}

void Portfolio::logPnLHistoryToCSV(const std::string &filename) const {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening portfolio PnL CSV file." << endl;
        return;
    }
    file << "DateTime,Stock,Quantity,RealizedPnL\n";
    for (const auto &record : pnlHistory) {
        file << record.dateTime << "," << record.stock << ","
             << record.quantity << "," << record.realizedPnL << "\n";
    }
    file.close();
    cout << "Portfolio PnL history saved to " << filename << endl;
}

void Portfolio::printAssetPerformance(const map<string, OrderBookStatistics>& marketStats) const {
    cout << "\nAsset Performance:" << endl;

    for (const auto &pair : holdings) {
        const string &stock = pair.first;
        const Holding &h = pair.second;

        if (marketStats.find(stock) != marketStats.end()) {
            const OrderBookStatistics &stats = marketStats.at(stock);

            double currentPrice = stats.midPrice;
            double aum = h.quantity * currentPrice;
            double unrealizedPnL = (currentPrice - h.averagePrice) * h.quantity;
 
            double assetRealizedPnL = 0.0;
            for (const auto &record : pnlHistory) {
                if (record.stock == stock) {
                    assetRealizedPnL += record.realizedPnL;
                }
            }
            double totalPnL = unrealizedPnL + assetRealizedPnL;
            
            cout << "Stock: " << stock << endl;
            cout << "  Quantity: " << h.quantity << ", Average Price: $" << h.averagePrice << endl;
            cout << "  Current Price: $" << currentPrice << ", AUM: $" << aum << endl;
            cout << "  Unrealized PnL: $" << unrealizedPnL << ", Realized PnL: $" << assetRealizedPnL 
                 << ", Total PnL: $" << totalPnL << "\n" << endl;
        } else {
            cout << "No market data for " << stock << endl;
        }
    }
}
    