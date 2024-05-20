//
// Created by Patryk Mulica    on 30/04/24.
//

#ifndef DRONE8_DRONE_H
#define DRONE8_DRONE_H

#include <atomic>
#include <string>
#include <future>
#include <random>
#include <thread>
#include <hiredis.h>

#include "../scanning_strategy/ScanningStrategy.h"
#include "../area/Area.hpp"
#include "../redis/redis.h"
#include "../config.h"



// Forward declaration
class Path;


/**
 * @brief Enum that represents the state of the drone
 */
enum class DroneState {
    ERROR,
    WORKING,
    CHARGING,
    READY,
    STOPPING,
};

/**
 * @brief Convert DroneState to string
 * @param state DroneState
 * @return string
 */
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

/**
 * @brief Convert string to DroneState
 * @param state string
 * @return DroneState
 */
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
 * @brief Struct that represents the data of the drone
 * @param id int ID of the drone, if id is -1, the data is invalid
 * @param x int X coordinate of the drone, if x is -1, the data is invalid
 * @param y int Y coordinate of the drone, if y is -1, the data is invalid
 * @param battery float Battery of the drone, if battery is -1.0f, the data is invalid
 * @param state DroneState State of the drone,
 *
 * @id If id is -1, the data is invalid
 *
 */
struct DroneData {
    int id = -1;
    int x = -1;
    int y = -1;
    float battery = -1.0f;
    DroneState state = DroneState::ERROR;

    bool operator==(const DroneData& other) const {
        return id == other.id;
    }
};


/**
 * @brief Drone class
 */
class Drone {
public:
    /* Constructors */
    Drone(unsigned int id);
    Drone(unsigned int id, unsigned int cc_id);

    /* Destructor */
    ~Drone();

    /* Getters */
    unsigned int getId() const;
    unsigned int getCCId() const;

    /* Setters */
    void setCCId(int cc_id);


    /* Methods */
    void start();
    void stop();
    void chargeDrone();



private:

    /* Data */
    /**
     * @brief ID of the drone
     */
    unsigned int id_; // ID del drone
    /**
     * @brief ID of the Control Center that controls the drone
     */
    unsigned int cc_id_{};

    /**
     * @brief Current data of the drone
     */
    DroneData current_data_{};
    int autonomy_ = Config::DRONE_STEPS;

    /* Redis */
    /**
     * @brief Redis context used to communicate with the Redis server

     */
    redisContext *ctx_;

    /**
     * @brief Atomic flag to stop the drone

     */
    static std::atomic<bool> stopFlag_;

    /* Methods */
    void listenCC();
    void followPath(const string& path);
    void sendDataToCC(bool changedState);
};



#endif //DRONE8_DRONE_H
