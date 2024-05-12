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
#include <cmath>
#include "../scanning_strategy/BasicStrategy.h"



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
    ControlCenter(unsigned int id, unsigned int num_drones, BasicStrategy *strategy, Area area); // TODO: basicStrategy cambiare con ScanningStrategy

    void setStrategy(BasicStrategy *strategy);


    void initDrones();

    void start();

    Coordinate getCCPosition();

    void stop();

    // Destructor
    ~ControlCenter();

private:

    void listenDrones();

    void updateArea(DroneData droneData);

    void sendPathsToDrones();

    void sendPath(unsigned int droneId, const Path& path);

    void insertCCLog(unsigned int droneId, unsigned int pathId);

    void insertDroneLog(DroneData droneData);

    const unsigned int id_;
    BasicStrategy* strategy_{};
    Area area_ = Area(0, 0);
    redisContext *sender_ctx_{};
    redisContext *listener_ctx_{};

    Con2DB conn_;

    unsigned int num_drones_;
    vector<DroneData> workingDrones_;
    vector<DroneData> chargingDrones_;
    vector<DroneData> readyDrones_;

    void handleSchedule(DroneSchedule schedule);
};

#endif //DRONE8_CONTROLCENTER_HPP