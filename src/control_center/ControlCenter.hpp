//
// Created by Patryk Mulica    on 30/04/24.
//

#ifndef DRONE8_CONTROLCENTER_HPP
#define DRONE8_CONTROLCENTER_HPP

#include "../area/Area.hpp"
#include "../con2db/pgsql.h"
#include "../config.h"
#include "../drone/Drone.h"
#include "../scanning_strategy/BasicStrategy.h"
#include "../scanning_strategy/ScanningStrategy.h"
#include <atomic>
#include <chrono>
#include <cmath>
#include <csignal>
#include <curl/curl.h>
#include <deque>
#include <hiredis/hiredis.h>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

enum class ControlCenterState {
  STARTING,
  SENDING_PATHS,
  WAITING,
  QUITTING
}; // TODO vedere se serve a qualcosa

using namespace std;

class ControlCenter {
public:
    /* Constructors */
    ControlCenter(unsigned int id, unsigned int num_drones);
    ControlCenter(unsigned int id, unsigned int num_drones,
                BasicStrategy *strategy,
                Area area); // TODO: basicStrategy cambiare con ScanningStrategy

    /* Destructor */
    ~ControlCenter();

    /* Getters */
    Coordinate getCCPosition();

    /* Setters */
    void setStrategy(BasicStrategy *strategy);

    /* Actions */
    void start();
    void stop();


private:
    // Connections
    Con2DB conn_;
    mutex query_mutex_;
    redisContext *listener_ctx_{};
    redisContext *sender_ctx_{};

    // CC Data
    const unsigned int id_;
    BasicStrategy *strategy_{};
    Area area_ = Area(0, 0);

    // For Handling drones
    unsigned int num_drones_;
    std::mutex lists_mutex_;
    std::deque<DroneData> workingDrones_;
    std::deque<DroneData> chargingDrones_;
    std::deque<DroneData> readyDrones_;

    std::atomic<bool> interrupt_{false};

    /* --- PRIVATE METHODS --- */
    /* Drones */
    void initDrones();
    void sendPaths();
    void handleSchedule(DroneSchedule schedule, redisContext *ctx);
    void listenDrones();
    void processMessage(Redis::Message message);

    /* Area */
    void printAreaStatus();

    /* DB */
    void executeQuery(const string &query);

    /* Log */
    void insertCCLog(unsigned int droneId, unsigned int pathId);
    void insertDroneLog(DroneData droneData);

    /* List Mutex */
    void addDroneToWorking(const DroneData &drone);
    void addDroneToCharging(const DroneData &drone);
    void addDroneToReady(const DroneData &drone);
    bool removeDroneFromWorking(const DroneData &droneToRemove);
    bool removeDroneFromCharging(const DroneData &droneToRemove);
    DroneData removeDroneFromReady();

};

#endif // DRONE8_CONTROLCENTER_HPP
