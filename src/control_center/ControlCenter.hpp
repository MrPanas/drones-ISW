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
#include "ThreadPool.cpp"

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
                ScanningStrategy *strategy,
                Area area); // TODO: basicStrategy cambiare con ScanningStrategy

    /* Destructor */
    ~ControlCenter();

    /* Getters */
    Coordinate getCCPosition();

    /* Setters */
    void setStrategy(ScanningStrategy *strategy);

    /* Actions */
    void start();
    void stop();


private:
    // Connections
    /**
     * @brief Connection to the database
     */
    Con2DB conn_;
    /**
     * @brief mutex for the database connection
     */
    mutex query_mutex_;
    /**
     * @brief Connection to the redis server for listening to the drones
     */
    redisContext *listener_ctx_{};
    /**
     * @brief Connection to the redis server for sending commands to the drones
     */
    redisContext *sender_ctx_{};

    // CC Data
    /**
     * @brief ID of the control center
     */
    const unsigned int id_;
    /**
     * @brief Strategy used by the control center to create the paths
     */
    ScanningStrategy *strategy_{};
    /**
     * @brief Area to be scanned
     */
    Area area_ = Area(-1, 0, 0);

    // For Handling drones
    /**
     * @brief Number of drones controlled by the control center
     */
    unsigned int num_drones_;
    /**
     * @brief mutex for accessing the drones lists
     */
    std::mutex lists_mutex_;
    /**
     * @brief List of drones currently working
     */
    std::deque<DroneData> workingDrones_;
    /**
     * @brief List of drones currently charging
     */
    std::deque<DroneData> chargingDrones_;
    /**
     * @brief List of drones ready to work
     */
    std::deque<DroneData> readyDrones_;

    /**
     * @brief Atomic variable used to interrupt the control center
     */
    std::atomic<bool> interrupt_{false};

    /* --- PRIVATE METHODS --- */
    /* Drones */
    void initDrones();
    void sendPaths();
    void handleSchedule(DroneSchedule schedule, redisContext *ctx);
    void listenDrones();
    void processMessage(string messageId, Redis::Message message);

    /* Area */
    void sendAreaToServer();

    /* DB */
    void executeQuery(const string &query);

    /* List Mutex */
    void addDroneToWorking(const DroneData &drone);
    void addDroneToCharging(const DroneData &drone);
    void addDroneToReady(const DroneData &drone);
    bool removeDroneFromWorking(const DroneData &droneToRemove);
    bool removeDroneFromCharging(const DroneData &droneToRemove);
    DroneData removeDroneFromReady();

};

#endif // DRONE8_CONTROLCENTER_HPP
