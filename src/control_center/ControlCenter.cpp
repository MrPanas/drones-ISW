//
// Created by Patryk Mulica    on 30/04/24.
//

#include <iostream>
#include <utility>
#include "ControlCenter.hpp"
#include "../redis/redis.h"


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
    ctx_ = redisConnect(REDIS_HOST, stoi(REDIS_PORT));
    if (ctx_ == NULL || ctx_->err) {
        if (ctx_) {
            std::cout << "Error: " << ctx_->errstr << std::endl;
            redisFree(ctx_);
        } else {
            std::cout << "Can't allocate redis context" << std::endl;
        }
    }

    // create cc group
    string stream = "cc_" + to_string(id_);
    string group = "CC_" + to_string(id_);

    createGroup(ctx_,stream, group, true);

    cout << "ControlCenter::ControlCenter: Successfully connected to redis server" << endl;

}

ControlCenter::ControlCenter(unsigned int id,
                             ScanningStrategy *strategy,
                             Area area) : ControlCenter(id){
    strategy_ = strategy;
    area_ = std::move(area);

}

void ControlCenter::setStrategy(ScanningStrategy *strategy) {
    strategy_ = strategy;
}

ControlCenter::~ControlCenter() {
    // Destory Group
    destroyGroup(ctx_, "cc_" + to_string(id_), "CC_" + to_string(id_));

    // Free the redis context
    redisFree(ctx_);

    cout << "ControlCenter::~ControlCenter: Successfully disconnected from redis server" << endl;

}

void ControlCenter::listenDrones() {
    const string stream = "cc_" + to_string(id_);
    const string group = "CC_" + to_string(id_);
    const string consumer = "CC_" + to_string(id_);



    while (true) {

        Response response = readMessageGroup(ctx_, group, consumer, stream, 0);


        string messageId = get<0>(response);
        cout << messageId << endl;
        if (messageId.empty()) {
            cerr << "ControlCenter::listenDrones: Error: Empty message" << endl;
            continue;
        }

        Message message = get<1>(response);
        // TODO: Process the message


    }

}

void ControlCenter::start() {



    cout << "ControlCenter::start: Starting Control Center" << endl;
    cout << "ControlCenter::start: Starting listenDrones thread" << endl;
    // Start the listenDrones thread
    thread listen(&ControlCenter::listenDrones, this);

    cout << "ControlCenter::start: Starting sendPathsToDrones thread" << endl;
    thread send(&ControlCenter::sendPathsToDrones, this);


    listen.join();
    send.join();
}

void ControlCenter::sendPath(unsigned int droneId, const Path& path) {
    const string stream = "stream_drone_" + to_string(droneId);

    string pathStr = path.toString();

    Message message;
    message["path"] = pathStr;
    sendMessage(ctx_, stream, message);
}

void ControlCenter::sendPathsToDrones() {
    // Open thread for each schedule
//    vector<DroneSchedule> schedules = strategy_->createSchedules(area_); // TODO: uncomment

    // DEBUG TODO: Remove after try
    Path p = Path();
    p.addDirection(Direction::NORTH, 3);
    p.addDirection(Direction::EAST, 2);
    vector<DroneSchedule> schedules = vector<DroneSchedule>();
    schedules.emplace_back(1, p, chrono::milliseconds(5000));
    // _________


    vector<thread> threads;
    threads.reserve(schedules.size());
    for (const DroneSchedule& schedule : schedules) {
        threads.emplace_back(&ControlCenter::handleSchedule, this, schedule);
    }

    for (thread &t : threads) {
        t.join();
    }
}


void ControlCenter::handleSchedule(DroneSchedule schedule) {
    int pathId = get<0>(schedule);
    Path path = get<1>(schedule);
    chrono::milliseconds nextSend = get<2>(schedule);

    while (true) {
        // Get a drone from readyDrones_
        DroneData droneData = readyDrones_.back();
        // Remove the drone from readyDrones_
        readyDrones_.pop_back();

        unsigned int droneId = droneData.id;

        sendPath(droneId, path);

        // Add drone to workingDrones_
        workingDrones_.push_back(droneData);

        // Wait for the next send
        this_thread::sleep_for(nextSend);
    }

}