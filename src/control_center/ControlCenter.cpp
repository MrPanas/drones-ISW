#include <iostream>
#include <utility>
#include "ControlCenter.hpp"
#include "../redis/redis.h"
#include <fstream>


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


ControlCenter::ControlCenter(unsigned int id, unsigned int num_drones) : id_(id), num_drones_(num_drones), conn_("localhost", "5432", "postgres", "postgres", "postgres") {
    area_ = Area(50, 50); // TODO: cancellare
    // Connect to redis
    sender_ctx_ = redisConnect(REDIS_HOST, REDIS_PORT);
    if (sender_ctx_ == NULL || sender_ctx_->err) {
        if (sender_ctx_) {
            std::cout << "ControlCenter::ControlCenter: Redis Error: " << sender_ctx_->errstr << std::endl;
            redisFree(sender_ctx_);
        } else {
            std::cout << "ControlCenter::ControlCenter: Redis: Can't allocate redis context" << std::endl;
        }
    }

    listener_ctx_ = redisConnect(REDIS_HOST, REDIS_PORT);
    if (listener_ctx_ == NULL || listener_ctx_->err) {
        if (listener_ctx_) {
            std::cout << "Error: " << listener_ctx_->errstr << std::endl;
            redisFree(listener_ctx_);
        } else {
            std::cout << "Can't allocate redis context" << std::endl;
        }
    }

    Redis::destroyGroup(sender_ctx_, "cc_" + to_string(id_), "CC_" + to_string(id_));
    Redis::destroyGroup(listener_ctx_, "cc_" + to_string(id_), "CC_" + to_string(id_));
    Redis::deleteStream(sender_ctx_, "cc_" + to_string(id_));

    // create cc group
    string stream = "cc_" + to_string(id_);
    string group = "CC_" + to_string(id_);

    Redis::createGroup(sender_ctx_, stream, group, true);

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
    Redis::destroyGroup(sender_ctx_, "cc_" + to_string(id_), "CC_" + to_string(id_));

    // Free the redis context
    redisFree(sender_ctx_);
    redisFree(listener_ctx_);

    cout << "ControlCenter::~ControlCenter: Successfully disconnected from redis server" << endl;

}

void ControlCenter::listenDrones() {
    const string stream = "cc_" + to_string(id_);
    const string group = "CC_" + to_string(id_);
    const string consumer = "CC_" + to_string(id_);

    cout << "ControlCenter::listenDrones: Listening for drones" << endl;

    while (true) {
        Redis::Response response = Redis::readMessageGroup(listener_ctx_, group, consumer, stream, 0);

        string messageId = get<0>(response);
        if (messageId.empty()) {
            cerr << "ControlCenter::listenDrones: Error: Empty message" << endl;
            continue;
        }

        Redis::Message message = get<1>(response);
        DroneData droneData = DroneData();
        droneData.id = stoi(message["id"]);
        // convert string to float
        droneData.x = stof(message["x"]);
        droneData.y = stof(message["y"]);
        droneData.battery = stof(message["battery"]);
        droneData.state = to_state(message["state"]);
        bool changedState = message["changedState"] == "true";

        if (changedState) {
            // insertDroneLog(droneData);
        }

        switch (droneData.state) {
            case DroneState::READY:
                if (!changedState) {
                    break;
                }
                // delete drone from chargingDrones
                for (int i = 0; i < chargingDrones_.size(); i++) {
                    if (chargingDrones_[i].id == droneData.id) {
                        // Swap the element with the last one
                        swap(chargingDrones_[i], chargingDrones_.back());
                        // Erase the last element (which is now the one to be removed)
                        chargingDrones_.pop_back();
                        break;
                    }
                }
                readyDrones_.push_back(droneData);
                break;
            case DroneState::WORKING:
                // call in another thread the function that updates the map
                {
                    // thread updateAreaThread(&ControlCenter::updateArea, this, droneData);
                    // updateAreaThread.detach(); // Detach the thread to allow it to run independently
                }
                break;
            case DroneState::CHARGING:
                if (!changedState) {
                    break;
                }
                // delete drone from workingDrones
                for (int i = 0; i < workingDrones_.size(); i++) {
                    if (workingDrones_[i].id == droneData.id) {
                        // Swap the element with the last one
                        swap(workingDrones_[i], workingDrones_.back());
                        // Erase the last element (which is now the one to be removed)
                        workingDrones_.pop_back();
                        break;
                    }
                }
                chargingDrones_.push_back(droneData);
                break;
            default:
                cerr << "ControlCenter::listenDrones: Error: Invalid state" << endl;
                break;
        }
    }
}

void ControlCenter::updateArea(DroneData droneData) {
    float x = droneData.x;
    float y = droneData.y;
     //TODO: rendere in futuro la distanza parametrica ???
    // check every point of the field that has less than 10 meters of distance from the drone

    // TODO mettere tutto questo in Area ?
    int start_x = std::max(static_cast<int>(x) - 10, 0);
    int start_y = std::max(static_cast<int>(y) - 10, 0);
    int end_x = std::min(static_cast<int>(std::ceil(x)) + 10, area_.getWidth());
    int end_y = std::min(static_cast<int>(std::ceil(y)) + 10, area_.getHeight());

    for (int i = start_x; i < end_x; i++) {
        for (int j = start_y; j < end_y; j++) {
            // calcola la distanza manhattan
            int distance = static_cast<int>(abs(x - i) + abs(y - j));
            if (distance <= 10) {
                area_.resetPointTimer(i, j);
            }
        }
    }
    // cout << area_.toString() << endl;
}

void ControlCenter::start() {
    cout << "ControlCenter::start: Starting Control Center" << endl;
    initDrones();

    // insertLog();

    // cout << "ControlCenter::start: Starting listenDrones thread" << endl;

    // Start the listenDrones thread
    thread listen(&ControlCenter::listenDrones, this);

    // cout << "ControlCenter::start: Starting sendPathsToDrones thread" << endl;
    thread send(&ControlCenter::sendPathsToDrones, this);


    listen.join();
    send.join();
}

void ControlCenter::initDrones() {
    const string stream = "cc_" + to_string(id_);
    const string group = "CC_" + to_string(id_);
    const string consumer = "CC_" + to_string(id_);

    // cout << "ControlCenter::initDrones: Waiting for " << num_drones_ << " drones" << endl;

    for (int i = 0; i < num_drones_; ++i) {
        Redis::Response response = Redis::readMessageGroup(sender_ctx_, group, consumer, stream, 0);
        string messageId = get<0>(response);
        if (messageId.empty()) {
            cerr << "ControlCenter::initDrones: Fatal Error: Empty message" << endl;
            break;
        }
        Redis::Message message = get<1>(response);
        DroneData droneData = DroneData();
        droneData.id = stoi(message["id"]);
        droneData.x = stof(message["x"]);
        droneData.y = stof(message["y"]);
        droneData.battery = stof(message["battery"]);
        droneData.state = to_state(message["state"]);

        readyDrones_.push_back(droneData);


        string query = "INSERT INTO drone (drone_id, battery, status) VALUES (" + to_string(droneData.id) + ", " + to_string(droneData.battery) + ", '" + to_string(droneData.state) + "') ON CONFLICT (drone_id) DO UPDATE SET battery = EXCLUDED.battery, status = EXCLUDED.status;"; // TODO: to_string(droneData.battery)

        // Converti la stringa in char *
        char *queryPtr = const_cast<char *>(query.c_str());

        // Esegui la query
        conn_.ExecSQLcmd(queryPtr);

        // cout << "ControlCenter::initDrones: Drone " << droneData.id << " is ready" << endl;
    }
}

void ControlCenter::sendPath(unsigned int droneId, const Path& path) {
    cout << "[1]ControlCenter::sendPath: Sending path to drone " << droneId << endl;
    const string stream = "stream_drone_" + to_string(droneId);

    string pathStr = path.toString();

    Redis::Message message;
    message["path"] = pathStr;

    Redis::sendMessage(sender_ctx_, stream, message);
    cout << "[2]ControlCenter::sendPath: Sending path to drone " << droneId << endl;
}

void ControlCenter::sendPathsToDrones() {
    // Open thread for each schedule
    vector<DroneSchedule> schedules = strategy_->createSchedules(area_);

    // DEBUG TODO: Remove after try
    /*
    Path p1 = Path();
    p1.addDirection(Direction::NORTH, 3);
    p1.addDirection(Direction::EAST, 2);
    // cout << "ControlCenter::sendPathsToDrones: path1: " << p1.toString() << endl;
    Path p2 = Path();
    p2.addDirection(Direction::SOUTH, 3);
    p2.addDirection(Direction::WEST, 2);
    vector<DroneSchedule> schedules = vector<DroneSchedule>();
    schedules.emplace_back(1, p1, chrono::milliseconds(10000));
    schedules.emplace_back(2, p2, chrono::milliseconds(10000));
     */
    // _________

    // ADD paths to DB
    for (DroneSchedule schedule : schedules) {
        string query = "INSERT INTO path (path_id, path) VALUES (" + to_string(get<0>(schedule)) + ", '" + get<1>(schedule).toString() + "') ON CONFLICT (path_id) DO UPDATE SET path = EXCLUDED.path;";
        conn_.ExecSQLcmd(const_cast<char *>(query.c_str()));
    }

    vector<thread> threads;
    threads.reserve(schedules.size());
    int i = 0;
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
        //cout << "ControlCenter::handleSchedule: Sending path " << pathId << " to drones" << endl;
        // Get a drone from readyDrones_
        DroneData droneData = readyDrones_.back(); // TODO mettere un if size == 0 per gestire errori

        cout << "ControlCenter::handleSchedule: ready drones: " << readyDrones_.size() << endl;

        // Remove the drone from readyDrones_
        readyDrones_.pop_back();

        unsigned int droneId = droneData.id;

        sendPath(droneId, path);

        //insertCCLog(droneId, pathId);

        // Add drone to workingDrones_
        workingDrones_.push_back(droneData);

        cout << "ControlCenter::handleSchedule: Sleeping for " << nextSend.count() << " milliseconds" << endl;

        // Wait for the next send
        this_thread::sleep_for(nextSend);
    }
}

void ControlCenter::insertCCLog(unsigned int droneId, unsigned int pathId) {
    string query = "INSERT INTO cc_log (drone_id, path_id) VALUES (" + to_string(droneId) + ", " + to_string(pathId) + ");";
    conn_.ExecSQLcmd(const_cast<char *>(query.c_str()));
}

void ControlCenter::insertDroneLog(DroneData data) {
    //conn_.ExecSQLcmd("INSERT INTO drone_log (id, message) VALUES (1, 'Drone " + to_string(data.id) + " started')");
    string query = "INSERT INTO drone_log (drone_id, path_id) VALUES (" + to_string(data.id) + ", 1);";
    conn_.ExecSQLcmd(const_cast<char *>(query.c_str()));
}

Coordinate ControlCenter::getCCPosition() {
    return Coordinate{area_.getWidth() / 2, area_.getHeight() / 2};
}