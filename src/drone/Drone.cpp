
#include <iostream>
#include <thread>
#include "Drone.h"

/**
 * @brief Global flag to stop the drone
 */
std::atomic<bool> Drone::stopFlag_{false};

/* --------- Constructors --------- */
/**
 * @brief Drone constructor. It initialize a connection to the Redis server and recreates a stream for the drone
 * @param id ID of the drone
 */
Drone::Drone(unsigned int id) : id_(id){
    current_data_ = {static_cast<int>(id_), -1, -1, 1, DroneState::READY};
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

/**
 * @brief Drone constructor. It initialize a connection to the Redis server and recreates a stream for the drone
 * Moreover, it sets the Control Center Id that controls the drone
 * @param id ID of the drone
 * @param cc_id ID of the Control Center that controls the drone
 */
Drone::Drone(unsigned int id, unsigned int cc_id) : Drone(id) {
    cc_id_ = cc_id;
}

/* --------- Destructor --------- */
/**
 * @brief Drone destructor. It destroys the stream and the group of the drone and closes the connection to the Redis server
 */
Drone::~Drone() {
    Redis::destroyGroup(ctx_, "stream_drone_" + to_string(id_), "Drone_" + to_string(id_));
    redisFree(ctx_);

//   cout << "Drone::~Drone: Drone " << id_ << " " << endl;
}

/* --------- Getters --------- */
/**
 * @brief Get the ID of the drone
 * @return ID of the drone
 */
unsigned int Drone::getId() const {
    return id_;
}

/**
 * @brief Get the ID of the Control Center that controls the drone
 * @return ID of the Control Center
 */
unsigned int Drone::getCCId() const {
    return cc_id_;
}


/* --------- Setters --------- */
/**
 * Set the Control Center ID
 * @param cc_id Control Center ID
 */
void Drone::setCCId(int cc_id) {
    cc_id_ = cc_id;
}


/* ------- Public methods ------- */
/**
 * Start the drone. First, it sends the current data to the Control Center, then it starts listening to the Control Centerr
 */
void Drone::start() {
    sendDataToCC(false);

    thread listen(&Drone::listenCC, this);
    listen.detach();
}

/**
 * @brief Charge drone to 100% autonomy in a random time between 2 and 3 hours
 */
void Drone::chargeDrone() {
    random_device rd; // Initialize a random number generator based on hardware
    mt19937 gen(rd()); // Create a random number generation engine with the seed from the random device

    // Create a uniform distribution between 2 and 3 hours (converted to milliseconds)
    std::uniform_int_distribution<> dis(Config::MINIMUM_CHARGING_TIME, Config::MAXIMUM_CHARGING_TIME); // 2 * 60 * 60 * 1000, 3 * 60 * 60 * 1000

    int charge_time = dis(gen); // Generate a random number between 2 and 3 hours

    if (autonomy_ > 0) {
        cout << "Drone " << id_ << " is charging for " << charge_time << " milliseconds, autonomy: " << autonomy_ << endl;
    }

    // 1 : charge_time = (1 - current_data_.battery) : x
    // x = charge_time * (1 - current_data_.battery)
    charge_time = (int)((float)charge_time * (1 - current_data_.battery));

    if (autonomy_ > 0) {
        cout << "Drone " << id_ << " is charging for " << charge_time << " milliseconds" << endl;
    }


    int sleep_time = static_cast<int>(charge_time * TIME_ACCELERATION);

    // check if stop
    for (int i = 0; i < sleep_time/1000; i++) {
        if (stopFlag_.load()) {
//            cout << "Drone " << id_ << " stopped charging" << endl;
            return;
        }
        this_thread::sleep_for(chrono::seconds(1));
    }

    // get the remaining milliseconds
    sleep_time = sleep_time % 1000;
    this_thread::sleep_for(chrono::milliseconds(sleep_time));


    current_data_.battery = 1.0;
    current_data_.state = DroneState::READY;
    autonomy_ = Config::DRONE_STEPS;

    sendDataToCC(true);
}

/* ------- Private methods ------- */
/**
 * @brief Listen to the Control Center. It reads the messages from the Control Center and executes the commands
 */
void Drone::listenCC() {
    string group = "Drone_" + to_string(id_);
    string consumer = "drone_" + to_string(id_);
    string stream = "stream_drone_" + to_string(id_);

    // cout << "Drone::listenCC: Drone " << id_ << " is listening to Control Center " << cc_id_ << endl;
    while (!stopFlag_.load()) {
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

        Redis::Message message = get<1>(res);
        if (message["command"] == "init-drone") {
            int x = stoi(message["cc-x"]);
            int y = stoi(message["cc-y"]);
            current_data_ = {current_data_.id, x, y, current_data_.battery, current_data_.state};
        }

        else if (message["command"] == "follow-path") {
            future<void> future = async(launch::async, &Drone::followPath, this, message["path"]);


        } else if (message["command"] == "stop") {

            current_data_.state = DroneState::STOPPING;
            stopFlag_.store(true);
//            cout << "Drone " << id_ << " stopping" << endl;
//            sendDataToCC(true);
            break;
        }

    }
//    cout << "Exit listenCC drone " << current_data_.id<< endl;

}

/**
 * @brief Follow the path received from the Control Center.
 * A step is executed every 2.4 seconds that corresponds to 20 meters at 8.33 m/s
 * The drone sends the current data to the Control Center every step
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

            // check if stop
            if (stopFlag_.load()) {
//                cout << "Drone " << id_ << " stopped" << endl;
                return ;
            }

            if (autonomy_ <= 0) {
                cout << "Drone " << id_ << " out of battery" << endl;
                sendDataToCC(false);
                return;
            }

            switch (dir) {
                case 'N':
                    current_data_.y--;
                    break;
                case 'S':
                    current_data_.y++;
                    break;
                case 'E':
                    current_data_.x++;
                    break;
                case 'W':
                    current_data_.x--;
                    break;
                default:
                    cerr << "Drone::followPath: Invalid direction" << endl;
            }

            current_data_.battery = (float)autonomy_/(float)Config::DRONE_STEPS;

            autonomy_--;



            sendDataToCC(false);
            // Il drone fa 1 metro in 0,12 secondi quindi a ogni istruzione fare il movimento e poi un time.sleep(0.12 seconds)
            int sleep_time = static_cast<int>(2400 * TIME_ACCELERATION);
            this_thread::sleep_for(chrono::milliseconds(sleep_time)); // TODO: mettere 2400
        }
    }
    // Print if autonomy != 0
    if (autonomy_ > 0) {
        cout << "Drone " << id_ << " finished the path with " << autonomy_ << " autonomy left" << endl;
    }

    current_data_.state = DroneState::CHARGING;
    sendDataToCC(true);
    chargeDrone();

}

/**
 * @brief Send the current data to the Control Center
 * @param changedState True if the state of the drone has changed
 */
void Drone::sendDataToCC(bool changedState) {
    string stream = "cc_" + to_string(cc_id_);

    Redis::Message message;
    message["id"] = to_string(id_);
    message["x"] = to_string(current_data_.x);
    message["y"] = to_string(current_data_.y);
    message["battery"] = to_string(current_data_.battery);
    message["state"] = to_string(current_data_.state);
    message["changedState"] = changedState ? "true" : "false";

    Redis::sendMessage(ctx_, stream, message);

}
