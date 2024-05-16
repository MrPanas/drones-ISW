
#include <iostream>
#include <thread>
#include "Drone.h"

Drone::Drone(unsigned int id) : id_(id){
    current_data_ = {static_cast<int>(id_), 150, 150, 1, DroneState::READY}; // TODO cambiare x,y con le coordinate del CC
    // cout << "Drone " << id_ << " created" << endl;
    ctx_ = redisConnect(REDIS_HOST, REDIS_PORT);
    if (ctx_ == NULL || ctx_->err) {
        if (ctx_) {
            cout << "Drone::Drone: Error: " << ctx_->errstr << endl;
            redisFree(ctx_);
        } else {
            cout << "Drone::Drone: Can't allocate redis context" << endl;
        }
    }
    Redis::destroyGroup(ctx_, "stream_drone_" + to_string(id_), "Drone_" + to_string(id_));
    Redis::deleteStream(ctx_, "stream_drone_" + to_string(id_));

    Redis::createGroup(ctx_, "stream_drone_" + to_string(id_), "Drone_" + to_string(id_) , true);

}

Drone::Drone(unsigned int id, unsigned int cc_id) : Drone(id) {
    cc_id_ = cc_id;
}

Drone::~Drone() {
    Redis::destroyGroup(ctx_, "stream_drone_" + to_string(id_), "Drone_" + to_string(id_));
    redisFree(ctx_);

    cout << "Drone " << id_ << " destroyed" << endl;
}

unsigned int Drone::getId() const {
    return id_;
}

/**
 * Set the Control Center ID
 * @param cc_id Control Center ID
 */
void Drone::setCCId(int cc_id) {
    cc_id_ = cc_id;
}

unsigned int Drone::getCCId() const {
    return cc_id_;
}

void Drone::start() {
    cout << "Drone " << id_ << " started" << endl;
    sendDataToCC(false);

    thread listen(&Drone::listenCC, this);

    // Attendere il completamento del thread
    listen.join();
}

void Drone::listenCC() {
    string group = "Drone_" + to_string(id_);
    string consumer = "drone_" + to_string(id_);
    string stream = "stream_drone_" + to_string(id_);

    // cout << "Drone::listenCC: Drone " << id_ << " is listening to Control Center " << cc_id_ << endl;
    while (true) {
        Redis::Response res = Redis::readMessageGroup(ctx_, group, consumer, stream, 0);
        string message_id = get<0>(res);
        if (message_id.empty()) {
            cout << "Drone::listenCC: Error reading message" << endl;
            continue;
        }

        // delete message
        long n_delete = Redis::deleteMessage(ctx_, stream, message_id);
        if (n_delete == -1) {
            cerr << "Drone::listenCC: Error: Can't delete message" << endl;
        }

        map<string, string> message = get<1>(res);

        future<void> future = async(launch::async, &Drone::followPath, this, message["path"]);
        future.wait();



    }
}

/**
 * Follow the path, every step takes 0.12 seconds and the drone moves 1 meter,
 * then it sends the new position to the Control Center
 * @param path Path to follow
 */
void Drone::followPath(const string &path) {
    if (current_data_.state != DroneState::READY) {
        cout << "Drone " << id_ << " executing a job or is charging" << endl;
        return;
    }
    current_data_.state = DroneState::WORKING;

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

            current_data_.battery = (float)autonomy_/(float)DRONE_AUTONOMY;

            autonomy_--;
            sendDataToCC(false);
            // Il drone fa 1 metro in 0,12 secondi quindi a ogni istruzione fare il movimento e poi un time.sleep(0.12 seconds)
            this_thread::sleep_for(chrono::milliseconds(2400)); // 0.12 * 1000 * 20
        }
    }
    // Print if autonomy != 0
    if (autonomy_ > 0) {
        cout << "Drone " << id_ << " finished the path with " << autonomy_ << " autonomy left" << endl;
    }

    current_data_.state = DroneState::CHARGING;
    sendDataToCC(true);
    chargeDrone(); // TODO: va bene così ho deve essere aperto in un nuovo thread

    // cout << "drone " << id_ << "current position: " << current_data_.x << " " << current_data_.y << endl;
}

void Drone::sendDataToCC(bool changedState) {
    string stream = "cc_" + to_string(cc_id_);

    Redis::Message message;
    message["id"] = to_string(id_);
    message["x"] = to_string(current_data_.x);
    message["y"] = to_string(current_data_.y);
    message["battery"] = to_string(current_data_.battery);
    message["state"] = to_string(current_data_.state);
    message["changedState"] = to_string(changedState);

    Redis::sendMessage(ctx_, stream, message);

}

void Drone::chargeDrone() {
    random_device rd; // Initialize a random number generator based on hardware
    mt19937 gen(rd()); // Create a random number generation engine with the seed from the random device

    // Create a uniform distribution between 2 and 3 hours (converted to milliseconds)
    std::uniform_int_distribution<> dis(7200000, 10800000); // 2 * 60 * 60 * 1000, 3 * 60 * 60 * 1000

    int charge_time = dis(gen); // Generate a random number between 2 and 3 hours

    this_thread::sleep_for(chrono::milliseconds(charge_time));

    current_data_.battery = 1;
    current_data_.state = DroneState::READY;
    autonomy_ = DRONE_AUTONOMY;

    sendDataToCC(true);
}