#ifndef BANKACCOUNT_H
#define BANKACCOUNT_H

#include <string>
#include <vector>

using namespace std;

// Structure to record a transaction (deposit or withdrawal)
struct Transaction {
    string dateTime;         // Date and time of the transaction
    string type;             // "Deposit" or "Withdrawal"
    double amount;                // Amount deposited or withdrawn
    double resultingBalance;      // Balance after the transaction
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
