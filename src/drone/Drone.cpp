
#include <iostream>
#include <thread>
#include "Drone.h"

Drone::Drone(unsigned int id) : id_(id){
    current_data_ = {id_, 0, 0, 100, DroneState::READY}; // TODO cambiare x,y con le coordinate del CC
    cout << "Drone " << id_ << " created" << endl;
    ctx_ = redisConnect(REDIS_HOST, stoi(REDIS_PORT));
    if (ctx_ == NULL || ctx_->err) {
        if (ctx_) {
            cout << "Drone::Drone: Error: " << ctx_->errstr << endl;
            redisFree(ctx_);
        } else {
            cout << "Drone::Drone: Can't allocate redis context" << endl;
        }
    }


    createGroup(ctx_, "stream_drone_" + to_string(id_), "Drone_" + to_string(id_) , true);


}

Drone::Drone(unsigned int id, unsigned int cc_id) : Drone(id) {
    cc_id_ = cc_id;
}

Drone::~Drone() {
    destroyGroup(ctx_, "stream_drone_" + to_string(id_), "Drone_" + to_string(id_));
    redisFree(ctx_);

    cout << "Drone " << id_ << " destroyed" << endl;
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
    sendDataToCC();

    thread listen(&Drone::listenCC, this);

    // Attendere il completamento del thread
    listen.join();
}

void Drone::listenCC() {
    cout << "Drone::listenCC: Drone " << id_ << " is listening to Control Center " << cc_id_ << endl;
    while (true) {
        Response res = readMessageGroup(ctx_, "Drone_" + to_string(id_), "drone_" + to_string(id_), "stream_drone_" + to_string(id_), 0);
        string message_id = get<0>(res);
        if (message_id.empty()) {
            cout << "Error reading message" << endl;
            continue;
        }

        map<string, string> message = get<1>(res);

        cout << "Message received: " << message["path"] << endl;

        // followPath(message["path"]); // TODO: chiamare questo metodo in un altro thread
        future<void> future = async(launch::async, &Drone::followPath, this, message["path"]);

    }
}

void Drone::followPath(const std::string &path) {
    if (current_data_.state != DroneState::READY) {
        cout << "Drone " << id_ << " executing a job" << endl;
        return;
    }

    cout << "Drone::followPath: Drone " << id_ << " is following the path" << endl;

    size_t i = 0;
    while (i < path.length()) {
        char dir = path[i++];
        string stepsStr;
        while (i < path.length() && isdigit(path[i])) {
            stepsStr += path[i++];
        } // format is <dir><steps>_<dir><steps>... this reads the direction and the steps

        i++;
        int steps = stoi(stepsStr);

        for (int j = 0; j < steps; j++) {
            switch (dir) {
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
                    cerr << "Drone::followPath: Invalid direction" << endl;
            }
            // TODO: sendDataToCC();
            // Il drone fa 1 metro in 0,12 secondi quindi a ogni istruzione fare il movimento e poi un time.sleep(0.12 seconds)
            this_thread::sleep_for(chrono::milliseconds(120));
        }
    }
    current_data_.state = DroneState::CHARGING;
    sendDataToCC();
    cout << "current drone position: " << current_data_.x << " " << current_data_.y << endl;
}

void Drone::sendDataToCC() {
    Message message;
    message["id"] = to_string(id_);
    message["x"] = to_string(current_data_.x);
    message["y"] = to_string(current_data_.y);
    message["battery"] = to_string(current_data_.battery);
    message["state"] = to_string(current_data_.state);

    sendMessage(ctx_, "cc_" + to_string(cc_id_), message);
}