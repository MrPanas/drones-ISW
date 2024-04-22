#ifndef DRONE_HPP
#define DRONE_HPP

#include <iostream>
#include <string>
#include "ControlCenter.hpp"

class Drone {
    public:
        // Costruttore
        Drone(int id, int cc_id);

        // Metodo per ottenere l'ID del drone
        int getId() const;

        int getControlCenterId() const;
        void handleCcRequests();
        void powerOn();

        void followPath();

    private:
        int id_;
        int cc_id_;
        std::string channelName_;
        redisContext *ctx_;
};

enum DroneStatus {
    WORKING,
    CHARGING,
    WAITING,
};

struct DroneData {
    int id;
    int x;
    int y;
    int battery;
    DroneStatus status;
};

#endif