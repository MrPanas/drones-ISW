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


ControlCenter::ControlCenter(unsigned int id) : id_(id) {
    // Connect to redis
//    ctx_ = redisConnect(REDIS_HOST,REDIS_PORT);



}


