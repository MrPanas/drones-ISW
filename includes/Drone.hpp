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

    std::vector<std::tuple<Direction, int>> requestPath();

    void followPath();

private:
    int id_;
    int cc_id_;
    RedisClient redisClient;
    std::string channelName_;
};

#endif
