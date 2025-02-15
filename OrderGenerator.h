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

using namespace std;

extern const vector<string> ASSETS;

double generateRandomNormal(double mean, double variance);
double generateRandomUniform(double lower, double upper);
double roundToTickSize(double value, double tickSize);
string generateRandomTimestamp();
void generateOrders(int nbAssets, const vector<int>& nbOrders,
                    const vector<double>& prices, const vector<double>& shortRatios = {0.1},
                    const string& outputFilename = "orders.csv");
#endif