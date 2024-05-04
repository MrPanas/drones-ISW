//
// Created by Patryk Mulica    on 30/04/24.
//

#ifndef DRONE8_DRONE_H
#define DRONE8_DRONE_H


#include <string>
#include <hiredis.h>
#include "../scanning_strategy/ScanningStrategy.h"

// Possible states of the drone
enum class DroneState {
    WORKING,
    CHARGING,
    WAITING,
};

struct DroneData {
    int id;
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
    Drone(int id);
    Drone(int id, int cc_id);

    int getId();

    void setCCId(int cc_id);

    int getCCId() const;


    void start();

    void stop();

    ~Drone();

private:
    int id_; // ID del drone
    int cc_id_;
    redisContext *ctx_;

    void listenCC();

    void followPath(Path path);

    void sendDataToCC();
};



#endif //DRONE8_DRONE_H
