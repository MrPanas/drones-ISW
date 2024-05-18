//
// Created by Patryk Mulica    on 30/04/24.
//

#ifndef DRONE8_DRONE_H
#define DRONE8_DRONE_H

#include <atomic>
#include <string>
#include <hiredis.h>
#include "../scanning_strategy/ScanningStrategy.h"
#include "../area/Area.hpp"
#include "../redis/redis.h"
#include <future>
#include <random>
#include "../scanning_strategy/config.h"


// Forward declaration
class Path;


// Possible states of the drone
enum class DroneState {
    WORKING,
    CHARGING,
    READY,
    STOPPING,
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
    return "UNKNOWN";
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
/**
 * Struct that represents the data of the drone
 *
 * @id If id is -1, the data is invalid

 */
struct DroneData {
    int id = -1;
    int x;
    int y;
    float battery;
    DroneState state;

    bool operator==(const DroneData& other) const {
        return id == other.id;
    }
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
    int autonomy_ = DRONE_AUTONOMY;
    static std::atomic<bool> stopFlag_;

    void listenCC();

    void followPath(const string& path);

    void sendDataToCC(bool changedState);
};



#endif //DRONE8_DRONE_H
