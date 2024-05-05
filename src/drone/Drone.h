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


// Forward declaration
class Path;


// Possible states of the drone
enum class DroneState {
    WORKING,
    CHARGING,
    READY,
};

struct DroneData {
    unsigned int id;
    int x;
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


    void start();

    void stop();

    ~Drone();

private:
    unsigned int id_; // ID del drone
    unsigned int cc_id_{};
    redisContext *ctx_;
    DroneData current_data_;

    void listenCC();

    void followPath(const string& path);

    void sendDataToCC();
};



#endif //DRONE8_DRONE_H
