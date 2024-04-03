#include "ControlCenter.hpp"
#include <iostream>

using namespace std;

ControlCenter::ControlCenter(int id) : id_(id), map(MapToScan()), algorithm(nullptr) { }

void ControlCenter::setAlgorithm(Algorithm* alg) {
    algorithm = alg;
}

void ControlCenter::setMap(MapToScan map) {
    this->map = map;
}

