//
// Created by Patryk Mulica    on 30/04/24.
//

#ifndef DRONE8_CONTROLCENTER_HPP
#define DRONE8_CONTROLCENTER_HPP

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <hiredis/hiredis.h>
#include <atomic>
#include "../scanning_strategy/ScanningStrategy.h"
#include "../area/Area.hpp"
#include "../drone/Drone.h"
#include "../con2db/pgsql.h"




enum class ControlCenterState {
    STARTING,
    SENDING_PATHS,
    WAITING,
    QUITTING
}; // TODO vedere se serve a qualcosa


using namespace std;




class ControlCenter {
public:
    ControlCenter(unsigned int id, unsigned int num_drones);
    ControlCenter(unsigned int id, unsigned int num_drones, ScanningStrategy *strategy, Area area);

    void setStrategy(ScanningStrategy *strategy);


    void initDrones();

    void start();

    void stop();

    // Destructor
    ~ControlCenter();

private:

    void listenDrones();

    void updateMap(DroneData droneData);

    void sendPathsToDrones();

    void sendPath(unsigned int droneId, const Path& path);

    void insertLog();

    const unsigned int id_;
    ScanningStrategy* strategy_{};
    Area area_ = Area(0, 0);
    redisContext *ctx_{};

    unsigned int num_drones_;
    vector<DroneData> workingDrones_;
    vector<DroneData> chargingDrones_;
    vector<DroneData> readyDrones_;

    PGconn *conn_{};

    void handleSchedule(DroneSchedule schedule);
};

#endif //DRONE8_CONTROLCENTER_HPP