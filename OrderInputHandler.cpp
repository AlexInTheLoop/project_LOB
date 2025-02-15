#include "OrderInputHandler.h"
#include <iostream>
#include <algorithm>
#include "OrderGenerator.h"

using namespace std;

OrderInputHandler::OrderInputHandler() {
    loadValidStocks();
}

void OrderInputHandler::loadValidStocks() {
    validStocks = ASSETS;
}

string OrderInputHandler::getOrderType() {
    string orderType;
    while (true) {
        cout << "Enter order type (BUY or SELL): ";
        cin >> orderType;

        transform(orderType.begin(), orderType.end(), orderType.begin(), ::toupper);
        if (orderType == "BUY" || orderType == "SELL")
            break;
        cout << "Invalid order type. Please enter BUY or SELL." << endl;
    }
    return orderType;
}

string OrderInputHandler::getStockSymbol() {
    string stock;
    while (true) {
        cout << "Enter stock symbol: ";
        cin >> stock;
        transform(stock.begin(), stock.end(), stock.begin(), ::toupper);
        bool found = false;
        for (const auto &s : validStocks) {
            if (s == stock) {
                found = true;
                break;
            }
        }
        if (found)
            break;
        cout << "Invalid stock symbol. Valid symbols are: ";
        for (const auto &s : validStocks)
            cout << s << " ";
        cout << endl;
    }
    return stock;
}

float OrderInputHandler::getFloatInput(const string &prompt) {
    float value;
    while (true) {
        cout << prompt;
        if (cin >> value)
            break;
        else {
            cout << "Invalid input. Please enter a number." << std::endl;
            cin.clear();
            cin.ignore(10000, '\n');
        }
    }
    return value;
}
