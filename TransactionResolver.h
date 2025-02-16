#ifndef TRANSACTION_RESOLVER_H
#define TRANSACTION_RESOLVER_H

#include <string>
#include "BankAccount.h"
#include "Portfolio.h"

std::string getCurrentDateTime();

void processBuyOrder(BankAccount &account, Portfolio &portfolio,
                     const std::string &stock, double quantity, double price);

void processSellOrder(BankAccount &account, Portfolio &portfolio,
                      const std::string &stock, double quantity, double price);

#endif