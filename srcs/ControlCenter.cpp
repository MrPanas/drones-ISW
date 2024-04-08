#include "ControlCenter.hpp"
#include <iostream>

using namespace std;

ControlCenter::ControlCenter(int id) : id_(id), redisClient_("localhost", 6379) {
    channelName = "cc_" + to_string(id_);
    redisClient_.sendCommand("SUBSCRIBE " + channelName);
}

void ControlCenter::setAlgorithm(Algorithm* alg) {
    algorithm = alg;
}


