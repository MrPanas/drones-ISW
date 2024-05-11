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

// TODO: ricarica del drone (lato drone) [FATTO]
// TODO: gestire che il cc toglie il drone da charging a ready e da working a charging [FATTO]
// TODO: il cc deve aggiornare la mappa appena gli arriva il messaggio del drone [FATTO]
// TODO: gestire chiusura di tutti i while true
// TODO: creare collegamento con il db

ControlCenter::ControlCenter(unsigned int id, unsigned int num_drones) : id_(id), num_drones_(num_drones), conn_("localhost", "5432", "postgres", "postgres", "postgres") {
    area_ = Area(50, 50); // TODO: cancellare
    // Connect to redis
    sender_ctx_ = redisConnect(REDIS_HOST, stoi(REDIS_PORT));
    if (sender_ctx_ == NULL || sender_ctx_->err) {
        if (sender_ctx_) {
            std::cout << "Error: " << sender_ctx_->errstr << std::endl;
            redisFree(sender_ctx_);
        } else {
            std::cout << "Can't allocate redis context" << std::endl;
        }
    }

    listener_ctx_ = redisConnect(REDIS_HOST, stoi(REDIS_PORT));
    if (listener_ctx_ == NULL || listener_ctx_->err) {
        if (listener_ctx_) {
            std::cout << "Error: " << listener_ctx_->errstr << std::endl;
            redisFree(listener_ctx_);
        } else {
            std::cout << "Can't allocate redis context" << std::endl;
        }
    }

    // create cc group
    string stream = "cc_" + to_string(id_);
    string group = "CC_" + to_string(id_);

    Redis::createGroup(sender_ctx_, stream, group, true);

    cout << "ControlCenter::ControlCenter: Successfully connected to redis server" << endl;

}

ControlCenter::ControlCenter(unsigned int id,
                             unsigned int num_drones,
                             ScanningStrategy *strategy,
                             Area area) : ControlCenter(id, num_drones){
    strategy_ = strategy;
    area_ = std::move(area);

}

void ControlCenter::setStrategy(ScanningStrategy *strategy) {
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

    while (true) {
        Redis::Response response = Redis::readMessageGroup(listener_ctx_, group, consumer, stream, 0);

        string messageId = get<0>(response);
        cout << messageId << endl;
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
            insertDroneLog(droneData);
        }

        cout <<"ControlCenter::listenDrones: Drone state: " << to_string(droneData.state) << endl;
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
                    cout << "ControlCenter::listenDrones: Drone " << droneData.id << " is working" << endl;
                    thread updateAreaThread(&ControlCenter::updateArea, this, droneData);
                    updateAreaThread.detach(); // Detach the thread to allow it to run independently
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
    cout << "ControlCenter::updateArea: Area updated" << endl;
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

        cout << "ControlCenter::initDrones: BATTERIA: " << droneData.battery << endl;

        std::string query = "INSERT INTO drone (drone_id, battery, status) VALUES (" + to_string(droneData.id) + ", " + to_string(1) + ", '" + to_string(droneData.state) + "');"; // TODO: to_string(droneData.battery)

        // Converti la stringa in char *
        char *queryPtr = const_cast<char *>(query.c_str());

        // Esegui la query
        conn_.ExecSQLcmd(queryPtr);

        // cout << "ControlCenter::initDrones: Drone " << droneData.id << " is ready" << endl;
    }
}

void ControlCenter::sendPath(unsigned int droneId, const Path& path) {
    const string stream = "stream_drone_" + to_string(droneId);

    string pathStr = path.toString();

    Redis::Message message;
    message["path"] = pathStr;
    cout << "ControlCenter::sendPath: Sending path " << pathStr << " to drone " << droneId << endl;

    Redis::sendMessage(sender_ctx_, stream, message);
    cout << "ControlCenter::sendPath: Path sent" << endl;
}

void ControlCenter::sendPathsToDrones() {
    // Open thread for each schedule
//    vector<DroneSchedule> schedules = strategy_->createSchedules(area_); // TODO: uncomment

    // DEBUG TODO: Remove after try
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
    // _________

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
        cout << "\n\nControlCenter::handleSchedule: joined while loop" << endl;
        //cout << "ControlCenter::handleSchedule: Sending path " << pathId << " to drones" << endl;
        // Get a drone from readyDrones_
        DroneData droneData = readyDrones_.back(); // TODO mettere un if size == 0 per gestire errori

        cout << "ControlCenter::handleSchedule: ready drones: " << readyDrones_.size() << endl;

        // Remove the drone from readyDrones_
        readyDrones_.pop_back();

        unsigned int droneId = droneData.id;

        sendPath(droneId, path); // BUG: non procede oltre qua

        // Add drone to workingDrones_
        workingDrones_.push_back(droneData);

        cout << "ControlCenter::handleSchedule: Sleeping for " << nextSend.count() << " milliseconds" << endl;

        // Wait for the next send
        this_thread::sleep_for(nextSend);
    }
}

void ControlCenter::insertCCLog() {
    // Connect to the database
    //Con2DB conn = Con2DB("localhost", "5432", "postgres", "postgres", "postgres");

    // Insert the log
    //conn.ExecSQLcmd("INSERT INTO log (id, message) VALUES (1, 'Control Center started')");

    while (true) {
        // Insert the log
        //conn.ExecSQLcmd("INSERT INTO log (id, message) VALUES (1, 'Control Center running')");

        // Sleep for 10 seconds
        this_thread::sleep_for(chrono::seconds());
    }
}

void ControlCenter::insertDroneLog(DroneData data) {
    //conn_.ExecSQLcmd("INSERT INTO drone_log (id, message) VALUES (1, 'Drone " + to_string(data.id) + " started')");
}