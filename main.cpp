#include <windows.h>
#undef byte

#include "order_generator.h"
#include "OrderBookManager.h"
#include "OrderBookSimulator.h"
#include "BankAccount.h"
#include "Portfolio.h"
#include "TransactionResolver.h"
#include "OrderInputHandler.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <csignal>
#include <cstdlib>

using namespace std;

// Global pointers for signal handling
BankAccount* g_userAccount = nullptr;
Portfolio* g_userPortfolio = nullptr;

// This function will be called when certain console events occur (e.g., close window, Ctrl+C).
BOOL WINAPI ConsoleHandler(DWORD dwCtrlType) {
    switch (dwCtrlType) {
        // Handle Ctrl+C or Close events gracefully
        case CTRL_C_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_SHUTDOWN_EVENT:
        case CTRL_LOGOFF_EVENT:
            if (g_userAccount && g_userPortfolio) {
                // Save data to CSV
                g_userAccount->logTransactionsToCSV("bank_transactions.csv");
                g_userPortfolio->logTradesToCSV("portfolio_trades.csv");
                g_userPortfolio->logPnLHistoryToCSV("portfolio_pnl.csv");
            }
            // Give some time to complete the file writes before the process ends
            Sleep(3000);
            return TRUE;

        // Return FALSE for events you do not handle
        default:
            return FALSE;
    }
}

int main() {
    // 1. Generate initial orders (stored in "transactions.csv")
    int nbAssets = 3;
    vector<int> nbOrders {100, 150, 200};
    vector<double> prices {150.0, 650.0, 300.0};
    vector<double> shortRatios {0.1, 0.2, 0.15};
    generateOrders(nbAssets, nbOrders, prices, shortRatios, "transactions.csv");

    // 2. Initialize OrderBookManager and process the orders
    OrderBookManager manager("transactions.csv");
    try {
        manager.loadOrders();
        manager.processOrders();
        cout << "\nInitial Order Book:" << endl;
        manager.displayOrderBooks();
        manager.saveOrderBooks("output");
    } catch (const exception &e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    // 3. Create BankAccount and Portfolio objects
    BankAccount userAccount(100000.0, "USD");
    Portfolio userPortfolio;

    // Assign global pointers for signal handling
    g_userAccount = &userAccount;
    g_userPortfolio = &userPortfolio;

    // Set console control handler
    if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
        cerr << "Error: Could not set control handler" << endl;
        return 1;
    }

    // 4. Create an instance of OrderInputHandler
    OrderInputHandler inputHandler;

    // 5. Interactive simulation loop
    cout << "\nStarting interactive simulation..." << endl;
    auto startTime = chrono::steady_clock::now();
    int simulationDuration = 3600; // in seconds

    while (true) {
        cout << "\n===== Current Order Book =====" << endl;
        for (const auto &statPair : manager.getStatistics()) {
            string asset = statPair.first;
            manager.displayOrderBook(asset);
        }

        cout << "\n----- Bank Account Status -----" << endl;
        cout << "Balance: " << userAccount.getBalance() << " USD" << endl;

        cout << "\n----- Portfolio -----" << endl;
        userPortfolio.printHoldings();
        userPortfolio.printGlobalPnL();

        // Print performance for each asset
        userPortfolio.printAssetPerformance(manager.getStatistics());

        cout << "\nWould you like to place a manual order? (y/n): ";
        char response;
        cin >> response;
        if (response == 'y' || response == 'Y') {
            // Collect order details
            string orderType = inputHandler.getOrderType();
            string stock = inputHandler.getStockSymbol();
            float price = inputHandler.getFloatInput("Enter the price: ");
            float quantity = inputHandler.getFloatInput("Enter the quantity: ");

            // Populate a new Order
            Order order;
            order.id = 9999;
            order.asset = stock;
            order.timestamp = getCurrentDateTime();
            order.dateTime = chrono::system_clock::now();
            order.price = price;
            order.quantity = quantity;
            order.totalAmount = price * quantity;
            order.type = orderType;
            order.isShortSell = false;

            manager.processNewOrder(order);

            // Update BankAccount and Portfolio
            if (orderType == "BUY") {
                processBuyOrder(userAccount, userPortfolio, stock, quantity, price);
            } else {
                processSellOrder(userAccount, userPortfolio, stock, quantity, price);
            }

            manager.displayOrderBook(stock);

            cout << "\n----- BANK ACCOUNT SUMMARY -----" << endl;
            cout << "Balance: " << userAccount.getBalance() << " USD" << endl;
        
            cout << "\n----- PORTFOLIO SUMMARY -----" << endl;
            userPortfolio.printHoldings();
            userPortfolio.printGlobalPnL();

            userPortfolio.printAssetPerformance(manager.getStatistics());
        }

        // Wait before next iteration
        this_thread::sleep_for(chrono::seconds(5));

        // End simulation after the specified duration
        auto currentTime = chrono::steady_clock::now();
        auto elapsed = chrono::duration_cast<chrono::seconds>(currentTime - startTime).count();
        if (elapsed >= simulationDuration) {
            break;
        }
    }

    // 6. Save final state to CSV files
    manager.saveOrderBooks("output");
    userAccount.logTransactionsToCSV("bank_transactions.csv");
    userPortfolio.logTradesToCSV("portfolio_trades.csv");
    userPortfolio.logPnLHistoryToCSV("portfolio_pnl.csv");

    return 0;
}
