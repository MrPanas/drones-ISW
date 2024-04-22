#ifndef CONTROLCENTER_HPP
#define CONTROLCENTER_HPP

#include "Drone.hpp"
#include "Algorithm.hpp"
#include <hiredis/hiredis.h>

class Algorithm;

class ControlCenter {
    public:
        ControlCenter(int id);

        void setAlgorithm(Algorithm* alg);

        void handleDroneRequests();
        void sendPath(int droneId, std::string& path);
        void powerOn(); // TODO: vedere se ha senso fare powerOn e powerOff
        void powerOff();

    private:
        const int id_;
        Algorithm* algorithm;
        std::string channelName;
        redisContext *ctx_;
};

#endif

