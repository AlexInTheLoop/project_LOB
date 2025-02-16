#include <windows.h>
#undef byte

#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <csignal>
#include <cstdlib>
#include <mutex>

// Project headers
#include "OrderGenerator.h"
#include "OrderBookManager.h"
#include "OrderBookSimulator.h"
#include "BankAccount.h"
#include "Portfolio.h"
#include "TransactionResolver.h"
#include "OrderInputHandler.h"

// Utiliser le namespace std
using namespace std;

// Global pointers for signal handling
BankAccount* g_userAccount  = nullptr;
Portfolio*   g_userPortfolio= nullptr;
OrderBookManager* g_manager = nullptr;

// Global mutex to protect console output from multiple threads
mutex g_consoleMutex;

// Windows console handler
BOOL WINAPI ConsoleHandler(DWORD dwCtrlType) {
    switch (dwCtrlType) {
        case CTRL_C_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_SHUTDOWN_EVENT:
        case CTRL_LOGOFF_EVENT:
            // Gracefully save logs if pointers exist
            if (g_userAccount && g_userPortfolio && g_manager) {
                {
                    lock_guard<mutex> lock(g_consoleMutex);
                    cout << "Closing... Saving final logs.\n";
                }
                g_userAccount->logTransactionsToCSV("bank_transactions.csv");
                g_userPortfolio->logTradesToCSV("portfolio_trades.csv");
                g_userPortfolio->logPnLHistoryToCSV("portfolio_pnl.csv");
                g_manager->saveOrderBooks("output");
            }
            Sleep(2000); // give time for file writes
            return TRUE;
        default:
            return FALSE;
    }
}

int main() {
    // 1) Generate initial orders
    int nbAssets = 3;
    vector<int>    nbOrders     {100, 150, 200};
    vector<double> prices       {150.0, 650.0, 300.0};
    vector<double> shortRatios  {0.1,  0.2,  0.15};

    string csvPath = "transactions.csv";
    generateOrdersAndReturn(nbAssets, nbOrders, prices, shortRatios, csvPath);

    // 2) Create OrderBookManager and load the newly generated orders
    OrderBookManager manager(csvPath);
    try {
        manager.loadOrders();
        manager.processOrders();
        {
            lock_guard<mutex> lock(g_consoleMutex);
            cout << "\nInitial Order Book:\n";
            manager.displayOrderBooks();  
        }
        manager.saveOrderBooks("output");
    } catch (const exception &e) {
        lock_guard<mutex> lock(g_consoleMutex);
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    // 3) Create BankAccount and Portfolio
    BankAccount userAccount(100000.0, "USD");
    Portfolio userPortfolio;

    // 4) Set up global pointers for the console handler
    g_userAccount   = &userAccount;
    g_userPortfolio = &userPortfolio;
    g_manager       = &manager;

    // 5) Set the console control handler
    if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
        lock_guard<mutex> lock(g_consoleMutex);
        cerr << "Error: Could not set control handler" << endl;
        return 1;
    }

    // 6) Create the simulator and run it in a background thread
    OrderBookSimulator simulator(manager);
    thread simThread([&simulator]() {
        simulator.simulateRealtime(3600); 
    });

    // 7) Prepare for user interaction
    OrderInputHandler inputHandler;
    {
        lock_guard<mutex> lock(g_consoleMutex);
        cout << "\nStarting interactive simulation in the main thread...\n";
    }
    auto startTime = chrono::steady_clock::now();
    int simulationDuration = 3600;

    // 8) Main user loop
    while (true) {
        {
            lock_guard<mutex> lock(g_consoleMutex);
            cout << "\n===== Current Order Book =====" << endl;
            manager.displayOrderBooks();

            cout << "\n----- Bank Account Status -----" << endl;
            cout << "Balance: " << userAccount.getBalance() << " USD" << endl;

            cout << "\n----- Portfolio -----" << endl;
            userPortfolio.printHoldings();
            userPortfolio.printGlobalPnL();
            userPortfolio.printAssetPerformance(manager.getStatistics());

            cout << "\nWould you like to place a manual order? (y/n): ";
        }
        char response;
        cin >> response;

        if (response == 'y' || response == 'Y') {
            // Collect order details
            {
                lock_guard<mutex> lock(g_consoleMutex);
                cout << "Placing an order...\n";
            }
            string orderType = inputHandler.getOrderType();
            string stock     = inputHandler.getStockSymbol();
            float price     = inputHandler.getFloatInput("Enter the price: ");
            float quantity  = inputHandler.getFloatInput("Enter the quantity: ");

            // Build a new Order
            Order order;
            order.id          = 9999;
            order.asset       = stock;
            order.timestamp   = getCurrentDateTime();
            order.dateTime    = chrono::system_clock::now();
            order.price       = price;
            order.quantity    = quantity;
            order.totalAmount = price * quantity;
            order.type        = orderType;
            order.isShortSell = false;

            // Update the OrderBook
            manager.processNewOrder(order);

            // Update BankAccount and Portfolio
            if (orderType == "BUY") {
                processBuyOrder(userAccount, userPortfolio, stock, quantity, price);
            } else {
                processSellOrder(userAccount, userPortfolio, stock, quantity, price);
            }

            // Show updated book for that stock (locked output)
            {
                lock_guard<mutex> lock(g_consoleMutex);
                manager.displayOrderBook(stock);

                cout << "\n----- BANK ACCOUNT SUMMARY -----\n";
                cout << "Balance: " << userAccount.getBalance() << " USD" << endl;

                cout << "\n----- PORTFOLIO SUMMARY -----\n";
                userPortfolio.printHoldings();
                userPortfolio.printGlobalPnL();
                userPortfolio.printAssetPerformance(manager.getStatistics());
            }
        }

        this_thread::sleep_for(chrono::seconds(5));

        // Check if we reached the user loop's end
        auto now = chrono::steady_clock::now();
        auto elapsed = chrono::duration_cast<chrono::seconds>(now - startTime).count();
        if (elapsed >= simulationDuration) {
            break;
        }
    }

    // 9) Join the simulator thread
    {
        lock_guard<mutex> lock(g_consoleMutex);
        cout << "\nMain user loop finished. Waiting for simulator to end...\n";
    }
    simThread.join();

    // 10) Save final state
    manager.saveOrderBooks("output");
    userAccount.logTransactionsToCSV("bank_transactions.csv");
    userPortfolio.logTradesToCSV("portfolio_trades.csv");
    userPortfolio.logPnLHistoryToCSV("portfolio_pnl.csv");

    {
        lock_guard<mutex> lock(g_consoleMutex);
        cout << "Simulation complete. Exiting...\n";
    }
    return 0;
}
