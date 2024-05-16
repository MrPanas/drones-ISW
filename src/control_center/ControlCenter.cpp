#include <iostream>
#include <utility>
#include "ControlCenter.hpp"
#include "../redis/redis.h"

#include <csignal>


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
 */

// TODO: gestire chiusura di tutti i while true
// TODO: a ogni cambio di stato del drone fare l'update nel db
// TODO: gestire la batteria del drone (Drone.cpp)
// TODO: Mandare posizione CC ai droni
// TODO: implementare che il drone si scarica mentre lavora





ControlCenter::ControlCenter(unsigned int id, unsigned int num_drones) : id_(id), num_drones_(num_drones) {





    listener_ctx_ = redisConnect(REDIS_HOST, REDIS_PORT);
    if (listener_ctx_ == NULL || listener_ctx_->err) {
        if (listener_ctx_) {
            std::cout << "Error: " << listener_ctx_->errstr << std::endl;
            redisFree(listener_ctx_);
        } else {
            std::cout << "Can't allocate redis context" << std::endl;
        }
    }


    Redis::destroyGroup(listener_ctx_, "cc_" + to_string(id_), "CC_" + to_string(id_));
    Redis::destroyGroup(listener_ctx_, "cc_" + to_string(id_), "CC_" + to_string(id_));
    Redis::deleteStream(listener_ctx_, "cc_" + to_string(id_));

    // create cc group
    string stream = "cc_" + to_string(id_);
    string group = "CC_" + to_string(id_);

    Redis::createGroup(listener_ctx_, stream, group, true);

    cout << "ControlCenter::ControlCenter: Successfully connected to redis server" << endl;

}

ControlCenter::ControlCenter(unsigned int id,
                             unsigned int num_drones,
                             BasicStrategy *strategy,
                             Area area) : ControlCenter(id, num_drones){
    strategy_ = strategy;
    area_ = std::move(area);



}

void ControlCenter::setStrategy(BasicStrategy *strategy) {
    strategy_ = strategy;
}

ControlCenter::~ControlCenter() {
    // Destory Group
    Redis::destroyGroup(listener_ctx_, "cc_" + to_string(id_), "CC_" + to_string(id_));

    // Free the redis context
    redisFree(listener_ctx_);

    cout << "ControlCenter::~ControlCenter: Successfully disconnected from redis server" << endl;

}

void ControlCenter::processMessage(Redis::Message message) {

    // Create a DroneData object from the message
    DroneData droneData = DroneData();
    droneData.id = stoi(message["id"]);
    droneData.x = stoi(message["x"]);
    droneData.y = stoi(message["y"]);
    droneData.battery = stof(message["battery"]);
    droneData.state = to_state(message["state"]);

    bool changedState = message["changedState"] == "true";



    if (droneData.state == DroneState::WORKING)
        updateArea(droneData);

    if (!changedState) {
        return;
    }
    // else changedState == true

    // insertDroneLog(droneData);

    switch (droneData.state) {
        case DroneState::READY:
            if (removeDroneFromCharging(droneData))
                addDroneToReady(droneData);
            break;
        case DroneState::WORKING:
            // when change to WORKING, it's handled by handleSchedule thread
            break;
        case DroneState::CHARGING:
            if (removeDroneFromWorking(droneData))
                addDroneToCharging(droneData);
            break;
        default:
            cerr << "ControlCenter::listenDrones: Error: Invalid state" << endl;
            break;
    }

}

void ControlCenter::listenDrones() {
    const string stream = "cc_" + to_string(id_);
    const string group = "CC_" + to_string(id_);
    const string consumer = "CC_" + to_string(id_);

    cout << "ControlCenter::listenDrones: Listening for drones" << endl;

    while (!interrupt_.load()) {
        vector<Redis::Response> responses = Redis::readGroupMessages(listener_ctx_, group, consumer, stream, 0, 0);

        for (const auto &response : responses) {

            string messageId = get<0>(response);

            if (messageId.empty()) {
                cerr << "ControlCenter::listenDrones: Error: Empty message" << endl;
                continue;
            }

            Redis::Message message = get<1>(response);

            processMessage(message);

            // Delete message from the stream
            long n_delete = Redis::deleteMessage(listener_ctx_, stream, messageId);
            if (n_delete == -1) {
                cerr << "ControlCenter::listenDrones: Error: Can't delete message" << endl;
            }

        }
    }
}



void ControlCenter::stop() {
    interrupt_.store(true);
}

void printAreaStatus(Area& area) {
    while (true) {

        area.printPercentage();
        // wait 10 seconds
        this_thread::sleep_for(chrono::seconds(10));
    }
}



void ControlCenter::start() {
    cout << "ControlCenter::start: Starting Control Center" << endl;
    initDrones();

    // insertLog();
    // print percentage each 10 seconds
    thread printAreaStatusThread(printAreaStatus, std::ref(area_));

    cout << "ControlCenter::start: Starting listenDrones thread" << endl;
    thread listen(&ControlCenter::listenDrones, this);

    cout << "ControlCenter::start: Starting sendPathsToDrones thread" << endl;
    thread send(&ControlCenter::sendPaths, this);

    // Wait for the threads to finish
    send.join();
    listen.join();

}

void ControlCenter::initDrones() {
    const string stream = "cc_" + to_string(id_);
    const string group = "CC_" + to_string(id_);
    const string consumer = "CC_" + to_string(id_);

    // cout << "ControlCenter::initDrones: Waiting for " << num_drones_ << " drones" << endl;

    for (int i = 0; i < num_drones_; ++i) {
        Redis::Response response = Redis::readMessageGroup(listener_ctx_, group, consumer, stream, 0);
        string messageId = get<0>(response);
        if (messageId.empty()) {
            cerr << "ControlCenter::initDrones: Fatal Error: Empty message" << endl;
            break;
        }
        Redis::Message message = get<1>(response);
        DroneData droneData = DroneData();
        droneData.id = stoi(message["id"]);
        droneData.x = stoi(message["x"]);
        droneData.y = stoi(message["y"]);
        droneData.battery = stof(message["battery"]);
        droneData.state = to_state(message["state"]);



//        readyDrones_.push_back(droneData);
        addDroneToReady(droneData);

        // insertDroneLog(droneData);

        // Update the drone status in the database


//        string query = "INSERT INTO drone (drone_id, battery, status) VALUES (" + to_string(droneData.id) + ", " + to_string(droneData.battery) + ", '" + to_string(droneData.state) + "') ON CONFLICT (drone_id) DO UPDATE SET battery = EXCLUDED.battery, status = EXCLUDED.status;"; // TODO: to_string(droneData.battery)
//
//        // Converti la stringa in char *
//        char *queryPtr = const_cast<char *>(query.c_str());
//
//        // Esegui la query
//        conn_.ExecSQLcmd(queryPtr);

        // cout << "ControlCenter::initDrones: Drone " << droneData.id << " is ready" << endl;

        // Delete message from the stream
        long n_delete = Redis::deleteMessage(listener_ctx_, stream, messageId);
        if (n_delete == -1) {
            cerr << "ControlCenter::listenDrones: Error: Can't delete message" << endl;
        }
    }

    assert(readyDrones_.size() == num_drones_);
}



void ControlCenter::sendPaths() {
    // Open thread for each schedule

    vector<DroneSchedule> schedules = strategy_->createSchedules(area_);


    cout << "ControlCenter::sendPathsToDrones: ready drones: " << readyDrones_.size() << endl;

    vector<thread> threads(schedules.size());

    for (const DroneSchedule& schedule : schedules) {
        redisContext *ctx = redisConnect(REDIS_HOST, REDIS_PORT);
        if (ctx == NULL || ctx->err) {
            if (ctx) {
                std::cerr << "ControlCenter::handleSchedule: Redis Error: " << ctx->errstr << std::endl;
                redisFree(ctx);

            } else {
                std::cerr << "ControlCenter::handleSchedule: Redis: Can't allocate redis context" << std::endl;
                return;
            }
        }


        threads.emplace_back(&ControlCenter::handleSchedule, this, schedule, ctx);

    }

    for (thread &t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }


}




void ControlCenter::handleSchedule(DroneSchedule schedule, redisContext *ctx) {
    int pathId = get<0>(schedule);
    Path path = get<1>(schedule);
    chrono::milliseconds nextSend = get<2>(schedule); // TODO: add RATIO_TIME

    string pathStr = path.toString();

    Redis::Message message;
    message["path"] = pathStr;

    // init redis context
//    redisContext *ctx = redisConnect(REDIS_HOST, REDIS_PORT);
//    if (ctx == NULL || ctx->err) {
//        if (ctx) {
//            std::cerr << "ControlCenter::handleSchedule: Redis Error: " << ctx->errstr << std::endl;
//            redisFree(ctx);
//            return;
//        } else {
//            std::cerr << "ControlCenter::handleSchedule: Redis: Can't allocate redis context" << std::endl;
//            return;
//        }
//    }

    // wait a moment


    // Send path to a ready drone every nextSend milliseconds
    while (!interrupt_.load()) {
        //cout << "ControlCenter::handleSchedule: Sending path " << pathId << " to drones" << endl;

        // Get a drone from readyDrones_
//        DroneData droneData = readyDrones_.back(); // TODO mettere un if size == 0 per gestire errori

        // Remove the drone from readyDrones_
//        readyDrones_.pop_back();

        DroneData droneData = removeDroneFromReady();
        if (droneData.id == -1) {
            cerr << "ControlCenter::handleSchedule: Error: No drone available" << endl;
            break;
//          TODO: Handle better this case
        }

        const string stream = "stream_drone_" + to_string(droneData.id);

        // Send the path to the drone
        cout << "[1]ControlCenter::sendPath: Sending path to drone "<< droneData.id << endl;
        Redis::sendMessage(ctx, stream, message);
        cout << "[2]ControlCenter::sendPath: Sending path to drone " << droneData.id << endl;

        addDroneToWorking(droneData);

        //insertCCLog(droneId, pathId); TODO: uncomment this line

        cout << "ControlCenter::handleSchedule: Sleeping for " << nextSend.count() << " milliseconds" << endl;

        // Wait for the next send
        this_thread::sleep_for(nextSend);

    }

    // Free the redis context
    redisFree(ctx);

}

void ControlCenter::insertCCLog(unsigned int droneId, unsigned int pathId) {
//    string query = "INSERT INTO cc_log (drone_id, path_id) VALUES (" + to_string(droneId) + ", " + to_string(pathId) + ");";
//    conn_.ExecSQLcmd(const_cast<char *>(query.c_str()));
}

void ControlCenter::insertDroneLog(DroneData data) {
    //conn_.ExecSQLcmd("INSERT INTO drone_log (id, message) VALUES (1, 'Drone " + to_string(data.id) + " started')");
//    string query = "INSERT INTO drone_log (drone_id, path_id) VALUES (" + to_string(data.id) + ", 1);";
//    conn_.ExecSQLcmd(const_cast<char *>(query.c_str()));
}

Coordinate ControlCenter::getCCPosition() {
    return Coordinate{area_.getWidth() / 2, area_.getHeight() / 2};
}


