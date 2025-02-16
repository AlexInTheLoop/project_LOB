#include "TransactionResolver.h"
#include <iostream>
#include <ctime>
#include <cstdio> 

using namespace std;

string getCurrentDateTime(){
    time_t now = time(0);
    tm *ltm = localtime(&now);

    char buffer[32];

    sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d",
        1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday,
        ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
return string(buffer);
}

void processBuyOrder(BankAccount &account, Portfolio &portfolio,
                    const string &stock, double quantity, double price){
    
    string dateTime = getCurrentDateTime();
    double totalCose = quantity * price;;

    if (!account.withdraw(totalCose, dateTime)){
        cout << "Order not processed. Insufficient funds." << endl;
        return;
    }

    portfolio.updateBuy(stock, quantity, price, dateTime);
}

void processSellOrder(BankAccount &account, Portfolio &portfolio,
        const std::string &stock, double quantity, double price) {
    
    string dateTime = getCurrentDateTime();
    portfolio.updateSell(stock, quantity, price, dateTime);
    double totalAmount = quantity * price;

    account.deposit(totalAmount, dateTime);
}