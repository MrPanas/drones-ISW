
#include <iostream>
#include <thread>
#include "Drone.h"

Drone::Drone(unsigned int id) : id_(id){
    current_data_ = {id_, 0, 0, 100, DroneState::WAITING}; // TODO cambiare x,y con le coordinate del CC
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
            continue;
        }

        map<string, string> message = get<1>(res);

        cout << "Message received: " << message["path"] << endl;

        followPath(message["path"]); // TODO: chiamare questo metodo e poi non aspettare la risposta
    }
}

void Drone::followPath(string path) {
    if (current_data_.state != DroneState::WAITING) {
        cout << "Drone " << id_ << " executing a job" << endl;
        return;
    }

    cout << "Drone " << id_ << " is following the path" << endl;

    for (char c : path) {
        switch (c) {
            case 'N':
                current_data_.y--;
                break;
            case 'S':
                current_data_.y++;
                break;
            case 'E':
                current_data_.x--;
                break;
            case 'W':
                current_data_.x++;
                break;
            default:
                std::cerr << "Carattere non valido nel percorso: " << c << std::endl;
        }
    }
    this_thread::sleep_for(chrono::milliseconds(120));
    // Il drone fa 1 metro in 0,12 secondi quindi a ogni istruzione fare il movimento e poi un time.sleep(0.12)
}