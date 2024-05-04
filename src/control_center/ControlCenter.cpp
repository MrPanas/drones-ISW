//
// Created by Patryk Mulica    on 30/04/24.
//

#include <iostream>
#include "ControlCenter.hpp"

/*
 * Control Center must:
 *
 * 1) Send the path to the drones
 * 2) Receive the drone streams
 * 3) Execute the scan
 * 4) Send the data to the server
 *
 * CC Must listen response from the drones
 * CC Must send the path to the drones
 *
 * How can do both?
 *
 */




//ControlCenter::ControlCenter(int id, ScanningStrategy *strategy) {
//    id_ = id;
//    strategy_ = strategy;
//    // Hello, I'm a control center with ID: 1
//    std::cout << "Hello, I'm a control center with ID: " << id_ << std::endl;
//    state = ControlCenterState::STARTING;
//}
//
//
//void ControlCenter::addDrone(const std::string &drone_stream) {
//    drone_streams_.push_back(drone_stream);
//}
//
//void ControlCenter::executeScan() const {
//    // Starting Scanning inside ControlCenter
//    std::cout << "Starting Scanning inside ControlCenter" << std::endl;
//
//    // Open a thread for each drone
//
//    // Comunicate to each drone to start scanning
//
//    // Send a path to each drone
//
//    // Wait for all drones to finish scanning
//
//    // Merge all the data from the drones
//
//    // Send the data to the server
//
//    // Close the thread for each drone
//
//    // Scanning the area using the strategy
//
//
//    strategy_->scanArea();
//
//    // Finished Scanning inside ControlCenter
//    std::cout << "Finished Scanning inside ControlCenter" << std::endl;
//}
//
//void ControlCenter::setStrategy(ScanningStrategy *strategy) {
//    strategy_ = strategy;
//}







