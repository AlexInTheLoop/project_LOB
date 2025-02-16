#ifndef ORDER_GENERATOR_H
#define ORDER_GENERATOR_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <set>
#include <cmath>
#include <iomanip>
#include <sstream>

#include "OrderBookManager.h"

extern const std::vector<std::string> ASSETS;

double generateRandomNormal(double mean, double variance);
double generateRandomUniform(double lower, double upper);
double roundToTickSize(double value, double tickSize);
std::string generateRandomTimestamp();
void generateOrders(int nbAssets, const std::vector<int>& nbOrders,
                    const std::vector<double>& prices, const std::vector<double>& shortRatios = {0.1},
                    const std::string& outputFilename = "orders.csv");
                    
std::vector<Order> generateOrdersAndReturn(
            int nbAssets,
            const std::vector<int>& nbOrders,
            const std::vector<double>& prices,
            const std::vector<double>& shortRatios = {0.1},
            const std::string& outputFilename= "orders.csv"
);

#endif
