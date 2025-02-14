#include "BankAccount.h"
#include <iostream>
#include <fstream>
#include <cstdlib>

using namespace std;

BankAccount::BankAccount(double initialBalance, const string &currency) 
    :balance(initialBalance), currency(currency) {}

bool BankAccount::deposit(double amount, const string &dateTime) {
    if (amount <= 0) return false;
    balance += amount;
    logTransaction("Deposit", amount, dateTime);
    cout << "Deposited"<< amount << currency << ". New balance: " << balance << currency << endl;
    return true;
}

bool BankAccount::withdraw(double amount, const std::string &dateTime) {
    if (amount <= 0) return false;
    if (amount > balance) {
        cout << "Insufficient funds. Cannot withdraw" << amount << currency << endl;
        return false;
    }
    balance -= amount;
    logTransaction("Withdrawal", amount, dateTime);
    cout << "Withdrawn" << amount << currency << ". New balance: " << balance << currency << endl;
    return true;
}

double BankAccount::getBalance() const {
    return balance;
}

void BankAccount:: logTransactionsToCSV(const std::string &filename) const{
    ofstream file(filename);
    if (!file.is_open()){
        cerr << "Error opening bank account CSV file." << endl;
        return;
    }
    file << "DateTime,Type,Amount,ResultingBalance\n";
    for (const auto &t:transactionHistory) {
        file << t.dateTime << "," << t.type << "," << t.amount << "," << t.resultingBalance << "\n";
    }
    file.close();
    cout << "Bank Account transaction logged to " << filename << endl;
}
void BankAccount::logTransaction(const std::string &type, double amount, const std::string &dateTime) {
    Transaction t = {dateTime, type, amount, balance};
    transactionHistory.push_back(t);
}