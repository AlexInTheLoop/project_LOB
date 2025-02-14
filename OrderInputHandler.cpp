#include "OrderInputHandler.h"
#include <iostream>
#include <algorithm>
#include "order_generator.h"

OrderInputHandler::OrderInputHandler() {
    loadValidStocks();
}

void OrderInputHandler::loadValidStocks() {
    validStocks = ASSETS;
}

std::string OrderInputHandler::getOrderType() {
    std::string orderType;
    while (true) {
        std::cout << "Enter order type (BUY or SELL): ";
        std::cin >> orderType;

        std::transform(orderType.begin(), orderType.end(), orderType.begin(), ::toupper);
        if (orderType == "BUY" || orderType == "SELL")
            break;
        std::cout << "Invalid order type. Please enter BUY or SELL." << std::endl;
    }
    return orderType;
}

std::string OrderInputHandler::getStockSymbol() {
    std::string stock;
    while (true) {
        std::cout << "Enter stock symbol: ";
        std::cin >> stock;
        std::transform(stock.begin(), stock.end(), stock.begin(), ::toupper);
        bool found = false;
        for (const auto &s : validStocks) {
            if (s == stock) {
                found = true;
                break;
            }
        }
        if (found)
            break;
        std::cout << "Invalid stock symbol. Valid symbols are: ";
        for (const auto &s : validStocks)
            std::cout << s << " ";
        std::cout << std::endl;
    }
    return stock;
}

float OrderInputHandler::getFloatInput(const std::string &prompt) {
    float value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value)
            break;
        else {
            std::cout << "Invalid input. Please enter a number." << std::endl;
            std::cin.clear();
            std::cin.ignore(10000, '\n');
        }
    }
    return value;
}
