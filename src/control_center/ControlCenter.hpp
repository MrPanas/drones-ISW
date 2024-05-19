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
  ControlCenter(unsigned int id, unsigned int num_drones);
  ControlCenter(unsigned int id, unsigned int num_drones,
                BasicStrategy *strategy,
                Area area); // TODO: basicStrategy cambiare con ScanningStrategy

  void setStrategy(BasicStrategy *strategy);

  void initDrones();

  void start();

  Coordinate getCCPosition();

  void stop();

  // Destructor
  ~ControlCenter();

private:
  void listenDrones();

  void sendPaths();
  void handleSchedule(DroneSchedule schedule, redisContext *ctx);

  void printAreaStatus();
  //    void updateArea(DroneData droneData);

  void insertCCLog(unsigned int droneId, unsigned int pathId);

  void insertDroneLog(DroneData droneData);

  const unsigned int id_;
  BasicStrategy *strategy_{};
  Area area_ = Area(0, 0);

  redisContext *listener_ctx_{};
  redisContext *sender_ctx_{};

  Con2DB conn_;

  unsigned int num_drones_;
  std::deque<DroneData> workingDrones_;
  std::deque<DroneData> chargingDrones_;
  std::deque<DroneData> readyDrones_;
  std::mutex lists_mutex_;
  std::atomic<bool> interrupt_{false};

  void addDroneToWorking(const DroneData &drone) {
    std::lock_guard<std::mutex> lock(lists_mutex_);
    workingDrones_.push_back(drone);
  }

  void addDroneToCharging(const DroneData &drone) {
    std::lock_guard<std::mutex> lock(lists_mutex_);
    chargingDrones_.push_back(drone);
  }

  void addDroneToReady(const DroneData &drone) {
    std::lock_guard<std::mutex> lock(lists_mutex_);
    readyDrones_.push_back(drone);
  }

  bool removeDroneFromWorking(const DroneData &droneToRemove) {
    std::lock_guard<std::mutex> lock(lists_mutex_);
    auto it =
        std::find(workingDrones_.begin(), workingDrones_.end(), droneToRemove);
    if (it != workingDrones_.end()) {
      workingDrones_.erase(it);
      return true;
    }
    return false;
  }

  bool removeDroneFromCharging(const DroneData &droneToRemove) {
    std::lock_guard<std::mutex> lock(lists_mutex_);
    auto it = std::find(chargingDrones_.begin(), chargingDrones_.end(),
                        droneToRemove);
    if (it != chargingDrones_.end()) {
      chargingDrones_.erase(it);
      return true;
    }
    return false;
  }

  DroneData removeDroneFromReady() {
    std::lock_guard<std::mutex> lock(lists_mutex_);
    if (readyDrones_.empty()) {
      // Se la lista è vuota, restituisci un drone vuoto
      return DroneData{};
    }
    DroneData drone = readyDrones_.front();
    readyDrones_.pop_front();
    return drone;
  }

  void processMessage(Redis::Message message);
};

#endif // DRONE8_CONTROLCENTER_HPP
