//
// Created by Patryk Mulica    on 30/04/24.
//

#ifndef DRONE8_DRONE_H
#define DRONE8_DRONE_H


#include <string>
#include <hiredis.h>
#include "../scanning_strategy/ScanningStrategy.h"
#include "../area/Area.hpp"
#include "../redis/redis.h"
#include <future>
#include <random>


// Forward declaration
class Path;


// Possible states of the drone
enum class DroneState {
    WORKING,
    CHARGING,
    READY,
};

inline string to_string(DroneState state) {
    switch (state) {
        case DroneState::WORKING:
            return "WORKING";
        case DroneState::CHARGING:
            return "CHARGING";
        case DroneState::READY:
            return "READY";
    }
}

inline DroneState to_state(const string& state) {
    if (state == "WORKING") {
        return DroneState::WORKING;
    } else if (state == "CHARGING") {
        return DroneState::CHARGING;
    } else if (state == "READY") {
        return DroneState::READY;
    }
    return DroneState::READY;
}

struct DroneData {
    unsigned int id;
    int x; // TODO: rendere x e y float
    int y;
    int battery;
    DroneState state;
};


/**
 * Drone class
 */
class Drone {
public:
    // Constructor
    Drone(unsigned int id);
    Drone(unsigned int id, unsigned int cc_id);

    unsigned int getId() const;

    void setCCId(int cc_id);

    unsigned int getCCId() const;

    void chargeDrone();

    void start();

    void stop();

    ~Drone();

private:
    unsigned int id_; // ID del drone
    unsigned int cc_id_{};
    redisContext *ctx_;
    DroneData current_data_{};

    void listenCC();

    void followPath(const string& path);

    void sendDataToCC();
};



#endif //DRONE8_DRONE_H
