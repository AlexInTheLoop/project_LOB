#include "order_generator.h"
#include "OrderBookManager.h"
#include "OrderBookSimulator.h"
#include <iostream>

using namespace std;

int main() {
    int nbAssets = 3;
    vector<int> nbOrders {100, 150, 200};
    vector<double> prices {150.0, 650.0, 300.0};
    vector<double> shortRatios {0.1, 0.2, 0.15};
    generateOrders(nbAssets, nbOrders, prices, shortRatios, "transactions.csv");
    
    OrderBookManager manager("transactions.csv");
    
    try {
        manager.loadOrders();
        manager.processOrders();
        
        cout << "\nCarnet(s) d'ordres :" << endl;
        manager.displayOrderBooks();
        manager.saveOrderBooks("output");
        
        cout << "\nDemarrage de la simulation sur tous les actifs..." << endl;
        
        OrderBookSimulator simulator(manager);
        simulator.simulateRealtime(3600);
        
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}