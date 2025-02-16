#ifndef BANKACCOUNT_H
#define BANKACCOUNT_H

#include <string>
#include <vector>

using namespace std;

struct Transaction {
    string dateTime;
    string type; 
    double amount;
    double resultingBalance;
};

class BankAccount {
public : 
    BankAccount(double initialBalance, const std::string &currency);
    bool deposit(double amount, const std::string &dateTime);
    bool withdraw(double amount, const std::string &dateTime);
    double getBalance() const;
    void logTransactionsToCSV(const std::string &filename) const;

private:
    double balance;
    string currency;
    vector<Transaction> transactionHistory;

    void logTransaction(const string &type, double amount, const string &dateTime);
};

#endif 
