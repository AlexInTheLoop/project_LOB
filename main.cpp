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

// Global pointers for signal handling
BankAccount* g_userAccount  = nullptr;
Portfolio*   g_userPortfolio= nullptr;
OrderBookManager* g_manager = nullptr;

// Global mutex to protect console output from multiple threads
std::mutex g_consoleMutex;

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
                    std::lock_guard<std::mutex> lock(g_consoleMutex);
                    std::cout << "Closing... Saving final logs.\n";
                }
                g_userAccount->logTransactionsToCSV("data/bank_transactions.csv");
                g_userPortfolio->logTradesToCSV("data/portfolio_trades.csv");
                g_userPortfolio->logPnLHistoryToCSV("data/portfolio_pnl.csv");
                g_manager->saveOrderBooks("output");
            }
            Sleep(2000); // give time for file writes
            return TRUE;
        default:
            return FALSE;
    }
}

// ----------------------------------------------------------------------
int main() {
    // 1) Generate initial orders
    int nbAssets = 3;
    std::vector<int>    nbOrders     {100, 150, 200};
    std::vector<double> prices       {150.0, 650.0, 300.0};
    std::vector<double> shortRatios  {0.1,  0.2,  0.15};

    std::string csvPath = "data/transactions.csv";
    generateOrdersAndReturn(nbAssets, nbOrders, prices, shortRatios, csvPath);

    // 2) Create OrderBookManager and load the newly generated orders
    OrderBookManager manager(csvPath);
    try {
        manager.loadOrders();
        manager.processOrders();
        {
            std::lock_guard<std::mutex> lock(g_consoleMutex);
            std::cout << "\nInitial Order Book:\n";
            manager.displayOrderBooks();  
        }
        manager.saveOrderBooks("data");
    } catch (const std::exception &e) {
        std::lock_guard<std::mutex> lock(g_consoleMutex);
        std::cerr << "Error: " << e.what() << std::endl;
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
        std::lock_guard<std::mutex> lock(g_consoleMutex);
        std::cerr << "Error: Could not set control handler" << std::endl;
        return 1;
    }

    // 6) Create the simulator and run it in a background thread
    //    so it keeps injecting random orders for 60 seconds (example).
    //    Adjust duration as you like (e.g., 3600 for an hour).
    OrderBookSimulator simulator(manager);
    std::thread simThread([&simulator]() {
        simulator.simulateRealtime(3600);  // e.g. 1 minute
    });

    // 7) Prepare for user interaction
    OrderInputHandler inputHandler;
    {
        std::lock_guard<std::mutex> lock(g_consoleMutex);
        std::cout << "\nStarting interactive simulation in the main thread...\n";
    }
    auto startTime = std::chrono::steady_clock::now();
    int simulationDuration = 3600;  // run user loop for 60 seconds, for demo

    // 8) Main user loop (no for-loop around assets; we call displayOrderBooks() directly)
    while (true) {
        {
            std::lock_guard<std::mutex> lock(g_consoleMutex);
            std::cout << "\n===== Current Order Book =====" << std::endl;
            manager.displayOrderBooks();

            std::cout << "\n----- Bank Account Status -----" << std::endl;
            std::cout << "Balance: " << userAccount.getBalance() << " USD" << std::endl;

            std::cout << "\n----- Portfolio -----" << std::endl;
            userPortfolio.printHoldings();
            userPortfolio.printGlobalPnL();
            userPortfolio.printAssetPerformance(manager.getStatistics());

            std::cout << "\nWould you like to place a manual order? (y/n): ";
        }
        char response;
        std::cin >> response;

        if (response == 'y' || response == 'Y') {
            // Collect order details
            {
                std::lock_guard<std::mutex> lock(g_consoleMutex);
                std::cout << "Placing an order...\n";
            }
            std::string orderType = inputHandler.getOrderType();
            std::string stock     = inputHandler.getStockSymbol();
            float price     = inputHandler.getFloatInput("Enter the price: ");
            float quantity  = inputHandler.getFloatInput("Enter the quantity: ");

            // Build a new Order
            Order order;
            order.id          = 9999;
            order.asset       = stock;
            order.timestamp   = getCurrentDateTime();
            order.dateTime    = std::chrono::system_clock::now();
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
                std::lock_guard<std::mutex> lock(g_consoleMutex);
                manager.displayOrderBook(stock);

                std::cout << "\n----- BANK ACCOUNT SUMMARY -----\n";
                std::cout << "Balance: " << userAccount.getBalance() << " USD" << std::endl;

                std::cout << "\n----- PORTFOLIO SUMMARY -----\n";
                userPortfolio.printHoldings();
                userPortfolio.printGlobalPnL();
                userPortfolio.printAssetPerformance(manager.getStatistics());
            }
        }

        // Sleep a bit so we don't spam the console
        std::this_thread::sleep_for(std::chrono::seconds(5));

        // Check if we reached the user loop's end
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
        if (elapsed >= simulationDuration) {
            break;
        }
    }

    // 9) Join the simulator thread
    {
        std::lock_guard<std::mutex> lock(g_consoleMutex);
        std::cout << "\nMain user loop finished. Waiting for simulator to end...\n";
    }
    simThread.join();

    // 10) Save final state
    manager.saveOrderBooks("output");
    userAccount.logTransactionsToCSV("data/bank_transactions.csv");
    userPortfolio.logTradesToCSV("data/portfolio_trades.csv");
    userPortfolio.logPnLHistoryToCSV("data/portfolio_pnl.csv");

    {
        std::lock_guard<std::mutex> lock(g_consoleMutex);
        std::cout << "Simulation complete. Exiting...\n";
    }
    return 0;
}
