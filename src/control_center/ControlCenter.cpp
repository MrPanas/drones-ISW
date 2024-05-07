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
 */

// TODO: ricarica del drone (lato drone) [FATTO]
// TODO: gestire che il cc toglie il drone da charging a ready e da working a charging [FATTO]
// TODO: il cc deve aggiornare la mappa appena gli arriva il messaggio del drone
// TODO: gestire chiusura di tutti i while true
// TODO: creare collegamento con il db

// TODO: forse è più efficiente avere una lista unica perché sarà una lista statica, non dovrà ricercare per poi cancellare nessun elemento e ci risparmiamo anche lo switch [I]


ControlCenter::ControlCenter(unsigned int id, unsigned int num_drones) : id_(id), num_drones_(num_drones) {
    cout << "ControlCenter::ControlCenter: Creating Control Center " << id_ << endl;
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
        droneData.x = stoi(message["x"]);
        droneData.y = stoi(message["y"]);
        droneData.battery = stoi(message["battery"]);
        droneData.state = to_state(message["state"]);

        // [I] TODO: con una lista unica ci risparmiamo tutto questo switch e di conseguenza il for e il delete (entrambi sono O(n) -> O(n^2))
        switch (droneData.state) {
            case DroneState::READY:
                // delete drone from chargingDrones
                for (int i = 0; i < chargingDrones_.size(); i++) {
                    if (chargingDrones_[i].id == droneData.id) {
                        chargingDrones_.erase(chargingDrones_.begin() + i);
                        break;
                    }
                }
                readyDrones_.push_back(droneData);
                break;
            case DroneState::WORKING:
                // call in another thread the function that updates the map
                // updateMap(droneData);
                {
                    thread updateMapThread(&ControlCenter::updateMap, this, droneData);
                    updateMapThread.detach(); // Detach the thread to allow it to run independently
                }
                break;
            case DroneState::CHARGING:
                // delete drone from workingDrones
                for (int i = 0; i < workingDrones_.size(); i++) {
                    if (workingDrones_[i].id == droneData.id) {
                        workingDrones_.erase(workingDrones_.begin() + i);
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

void ControlCenter::start() {
    cout << "ControlCenter::start: Starting Control Center" << endl;
    // TODO: aggiungere una funzione dove vengono aggiunti tutti i droni
    initDrones();

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
        droneData.x = stoi(message["x"]);
        droneData.y = stoi(message["y"]);
        droneData.battery = stoi(message["battery"]);
        droneData.state = to_state(message["state"]);

        readyDrones_.push_back(droneData);

        // cout << "ControlCenter::initDrones: Drone " << droneData.id << " is ready" << endl;
    }
    cout << "ControlCenter::initDrones: There are " << readyDrones_.size() << " drones ready" << endl;

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
        DroneData droneData = readyDrones_.back(); // TODO mettere un if size == 0

        cout << "ControlCenter::handleSchedule: ready drones: " << readyDrones_.size() << endl;

        // cout << "ControlCenter::handleSchedule: Drone " << droneData.id << " is READY" << endl;
        // Remove the drone from readyDrones_
        readyDrones_.pop_back();

        unsigned int droneId = droneData.id;

        sendPath(droneId, path); // BUG: non procede oltre qua

        // Add drone to workingDrones_
        workingDrones_.push_back(droneData);

        // cout << "ControlCenter::handleSchedule: Drone " << droneData.id << " is WORKING" << endl;

        cout << "ControlCenter::handleSchedule: Sleeping for " << nextSend.count() << " milliseconds" << endl;

        // Wait for the next send
        this_thread::sleep_for(nextSend);
    }
}

void ControlCenter::updateMap(DroneData droneData) {
    // TODO: far diventare x,y dei float prima di implementare
}