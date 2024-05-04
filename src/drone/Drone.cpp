
#include <iostream>
#include "Drone.h"

Drone::Drone(unsigned int id) : id_(id),  state_(DroneState::WAITING){
    ctx_ = redisConnect(REDIS_HOST, stoi(REDIS_PORT));
    if (ctx_ == NULL || ctx_->err) {
        if (ctx_) {
            cout << "Error: " << ctx_->errstr << endl;
            redisFree(ctx_);
        } else {
            cout << "Can't allocate redis context" << endl;
        }
    }

    createGroup(ctx_, "stream_drone_" + to_string(id_), "Drone_" + to_string(id_) , true);
}

Drone::Drone(unsigned int id, unsigned int cc_id) : Drone(id) {
    cc_id_ = cc_id;
}

unsigned int Drone::getId() const {
    return id_;
}

void Drone::setCCId(int cc_id) {
    cc_id_ = cc_id;
}

unsigned int Drone::getCCId() const {
    return cc_id_;
}

void Drone::start() {
    listenCC();
}

void Drone::listenCC() {
    cout << "Drone " << id_ << " is listening to Control Center " << cc_id_ << endl;
    while (true) {
        Response res = readMessageGroup(ctx_, "drone_" + to_string(id_), "drone_" + to_string(id_), "Drone_" + to_string(id_), 0);
        string message_id = get<0>(res);
        if (message_id.empty()) {
            cout << "Error reading message" << endl;
        }
        map<string, string> message = get<1>(res);

        cout << "Message received: " << message["path"] << endl;

        followPath(message["path"]);
    }
}

void Drone::followPath(string path) {
    if (state_ != DroneState::WAITING) {
        cout << "Drone " << id_ << " executing a job" << endl;
        return;
    }

    cout << "Drone " << id_ << " is following the path" << endl;

    // TODO: Il drone fa 1 metro in 0,12 secondi quindi a ogni istruzione fare il movimento e poi un time.sleep(0.12)
}