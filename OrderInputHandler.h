#ifndef ORDER_INPUT_HANDLER_H
#define ORDER_INPUT_HANDLER_H

#include <string>
#include <vector>

class OrderInputHandler {
public:
    OrderInputHandler();
    std::string getOrderType(); 
    std::string getStockSymbol();  
    float getFloatInput(const std::string &prompt);

private:
    std::vector<std::string> validStocks;
    void loadValidStocks();
};

#endif
