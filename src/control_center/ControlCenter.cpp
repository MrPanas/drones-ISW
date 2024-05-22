#include <iostream>
#include <utility>
#include "ControlCenter.hpp"

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

// TODO: documentare il codice

/* --------------- PUBLIC METHODS --------------- */
/* ------ Constructors ------ */

/**
 * @brief Construct a new Control Center with the given id and number of drones. \n
 * It also connects to the redis server and postgres database. \n
 * Moreover, it recreates his group and stream in the redis server.
 * @param id unsigned int that represents the id of the control center.
 * @param num_drones  unsigned int that represents the number of drones that the control center must wait for.
 */
ControlCenter::ControlCenter(unsigned int id, unsigned int num_drones) : id_(id), num_drones_(num_drones), conn_("localhost", "5432", "postgres", "postgres", "postgres") {
    listener_ctx_ = redisConnect(REDIS_HOST, REDIS_PORT);
    if (listener_ctx_ == NULL || listener_ctx_->err) {
        if (listener_ctx_) {
            cout << "Error: " << listener_ctx_->errstr << endl;
            redisFree(listener_ctx_);
        } else {
            cout << "Can't allocate redis context" << endl;
        }
    }
    sender_ctx_ = redisConnect(REDIS_HOST, REDIS_PORT);
    if (sender_ctx_ == NULL || sender_ctx_->err) {
        if (sender_ctx_) {
            cout << "Error: " << sender_ctx_->errstr << endl;
            redisFree(sender_ctx_);
        } else {
            cout << "Can't allocate redis context" << endl;
        }
    }


    Redis::destroyGroup(listener_ctx_, "cc_" + to_string(id_), "CC_" + to_string(id_));

    Redis::deleteStream(listener_ctx_, "cc_" + to_string(id_));

    // create cc group
    string stream = "cc_" + to_string(id_);
    string group = "CC_" + to_string(id_);

    Redis::createGroup(listener_ctx_, stream, group, true);

    cout << "ControlCenter::ControlCenter: Successfully connected to redis server" << endl;

}

/**
 * @brief Construct a new Control Center with the given id, number of drones, strategy and area. \n
 * It also connects to the redis server and postgres database. \n
 * Moreover, it recreates his group and stream in the redis server. \n
 * It also inserts itself and the area in the database.
 * @param id unsigned int that represents the id of the control center.
 * @param num_drones unsigned int that represents the number of drones that the control center must wait for.
 * @param strategy BasicStrategy used to compute the paths for the drones.
 * @param area Area that represents the area where the drones must scan.
 */
ControlCenter::ControlCenter(unsigned int id,
                             unsigned int num_drones,
                             ScanningStrategy *strategy,
                             Area area) : ControlCenter(id, num_drones){
    strategy_ = strategy;
    area_ = std::move(area);

    // Insert the area in the database
    string query = "INSERT INTO area (id, width, height, point_expiration_time) VALUES (" +
                    to_string(area_.getId()) + ", " +
                    to_string(area_.getWidth()) + ", " +
                     to_string(area_.getHeight()) + ", " +
                     to_string(Config::POINT_EXPIRATION_TIME) + ") " +
                     "ON CONFLICT (id) DO UPDATE SET width = EXCLUDED.width, height = EXCLUDED.height, point_expiration_time = EXCLUDED.point_expiration_time;";
    executeQuery(query);

    // Insert the control center in the database
    query = "INSERT INTO control_center (id, area_id) VALUES (" +
            to_string(id_) + ", " +
            to_string(area_.getId()) + ") ON CONFLICT (id) DO UPDATE SET area_id = EXCLUDED.area_id;";
    executeQuery(query);

    query = "INSERT INTO session (cc_id, start_time) VALUES (" +
            to_string(id_) + ", NOW());";
    executeQuery(query);

}

/* ------ Destructor ------ */
/**
 * @brief Destroy the Control Center object. \n
 * It destroys the group and stream in the redis server and frees the redis context.
 */
ControlCenter::~ControlCenter() {
    // Destroy Group
    Redis::destroyGroup(listener_ctx_, "cc_" + to_string(id_), "CC_" + to_string(id_));

    // Free the redis context
    redisFree(listener_ctx_);

    string query = "UPDATE session SET end_time = NOW() WHERE cc_id = " + to_string(id_) + ";";
    executeQuery(query);

    cout << "ControlCenter::~ControlCenter: Successfully disconnected from redis server" << endl;
}


/* ------ Getters ------ */
/**
 * @brief Get the position of the Control Center in the area.
 * @return Coordinate that represents the position of the Control Center.
 */
Coordinate ControlCenter::getCCPosition() {
    return Coordinate{area_.getWidth() / 2, area_.getHeight() / 2};
}


/* ------ Setters ------ */
/**
 * @brief Set the strategy of the Control Center.
 * @param strategy BasicStrategy used to compute the paths for the drones
 */
void ControlCenter::setStrategy(ScanningStrategy *strategy) {
    strategy_ = strategy;
}


/* ------ Actions ------ */
/**
 * @brief Start the Control Center. \n
 * It initializes the drones, starts the threads to listen for the drones and send the paths to the drones. \n
 * It also starts a thread to print the area status every 10 seconds.
 */
void ControlCenter::start() {
    cout << "ControlCenter::start: Starting Control Center" << endl;
    initDrones();


    // print percentage each 10 seconds
    thread printAreaStatusThread(&ControlCenter::sendAreaToServer, this);

    cout << "ControlCenter::start: Starting listenDrones thread" << endl;
    thread listen(&ControlCenter::listenDrones, this);

    cout << "ControlCenter::start: Starting sendPathsToDrones thread" << endl;
    thread send(&ControlCenter::sendPaths, this);

    // Wait for the threads to finish
    printAreaStatusThread.join();
    cout << "ControlCenter::start: sendAreaToServer thread finished" << endl;

    send.join();
    cout << "ControlCenter::start: sendPaths thread finished" << endl;

    listen.join();
    cout << "ControlCenter::start: listenDrones thread finished" << endl;

    cout << "ControlCenter::start: Stopped Control Center" << endl;
}

/**
 * @brief Stop the Control Center. \n
 * It sends a stop message to all the drones and a quit message to itself. \n
 */
void ControlCenter::stop() {
    std::lock_guard<std::mutex> lock(lists_mutex_);
    cout << "ControlCenter::stop: Stopping Control Center" << endl;
    interrupt_.store(true);

    // Send to all drones to stop

    // merge ready, working, charging drones to allDrones

    // create allDrones
    std::deque<DroneData> allDrones;
    allDrones.insert(allDrones.end(), readyDrones_.begin(), readyDrones_.end());
    allDrones.insert(allDrones.end(), workingDrones_.begin(), workingDrones_.end());
    allDrones.insert(allDrones.end(), chargingDrones_.begin(), chargingDrones_.end());


    cout << "ControlCenter::stop: Stopping " << allDrones.size() << " drones" << endl;
    for (const DroneData &drone: allDrones) {
        //        cout << "ControlCenter::stop: Stopping drone " << drone.id << endl;
        string stream = "stream_drone_" + to_string(drone.id);
        Redis::Message message;
        message["command"] = "stop";
        Redis::sendMessage(sender_ctx_, stream, message);
    }

    // send to cc quit message
    string stream = "cc_1"; // TODO: check if it's correct
    Redis::Message message;
    message["command"] = "quit";
    Redis::sendMessage(sender_ctx_, stream, message);
}


/* ---------------- PRIVATE METHODS ---------------- */

/* ------ Drones ------ */
/**
 * @brief Initialize the drones. \n
 * It waits for the drones to send their position and state. \n
 * It sends the position of the Control Center to the drones. \n
 * It updates the database with the drones' information.
 */
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


        // Send the CC position to the drone
        string drone_stream = "stream_drone_" + to_string(droneData.id);
        Redis::Message init_message;

        int x = getCCPosition().x;
        int y = getCCPosition().y;
        init_message["command"] = "init-drone";
        init_message["cc-x"] = to_string(x);
        init_message["cc-y"] = to_string(y);

        Redis::sendMessage(listener_ctx_, drone_stream, init_message);


        // insertDroneLog(droneData);

        // Update the drone status in the database


        string query = "INSERT INTO drone (id, battery, status, cc_id) VALUES (" +
                       to_string(droneData.id) + ", " +
                       to_string(droneData.battery) + ", '" +
                       to_string(droneData.state) + "', " + // Aggiunto il singolo apice mancante qui
                       to_string(id_) + ") ON CONFLICT (id) " +
                       "DO UPDATE SET battery = EXCLUDED.battery, " +
                       "status = EXCLUDED.status;";
        executeQuery(query);

        // cout << "ControlCenter::initDrones: Drone " << droneData.id << " is ready" << endl;

        // Delete message from the stream
        long n_delete = Redis::deleteMessage(listener_ctx_, stream, messageId);
        if (n_delete == -1) {
            cerr << "ControlCenter::listenDrones: Error: Can't delete message" << endl;
        }
    }

    cout << "There are " << readyDrones_.size() << " drones ready" << endl;

    assert(readyDrones_.size() == num_drones_);
}

/**
 * @brief Send the paths to the drones. \n
 * Takes the schedules/path from the strategy, connects to redis and opens a thread for each schedule.\n
 * Than waits threads to finish.
 */
void ControlCenter::sendPaths() {
    // Open thread for each schedule

    vector<DroneSchedule> schedules = strategy_->createSchedules(area_);

    // MONITOR
    if (schedules.empty()) {
        cerr << "ControlCenter::sendPathsToDrones: Error: No schedules created" << endl;
        string query = "INSERT INTO monitor_failure (cc_id, failure, message, time) VALUES (" +
                        to_string(id_) + ", " +
                       "'DRONE_AUTONOMY', " +
                       "'Drone is initialized with too low autonomy', " +
                       "NOW());";
        executeQuery(query);
    }

    // minimum drones needed to scan the area
    // MONITOR
    unsigned int min_drones = schedules.size() * (10800000 / Config::POINT_EXPIRATION_TIME); // schedules * (3h/5min)
    cout << "ControlCenter::sendPathsToDrones: minimum drones needed: " << min_drones << endl;
    if (min_drones > num_drones_) {
        cerr << "ControlCenter::sendPathsToDrones: Error: Not enough drones to scan the area" << endl;
        string query = "INSERT INTO monitor_failure (cc_id, failure, message, time) VALUES (" +
                          to_string(id_) + ", " +
                          "'NUM_DRONES', " +
                          "'Not enough drones to scan the area, minimum drones needed: " + to_string(min_drones) + ", available drones: " + to_string(num_drones_) + "', " +
                          "NOW());";

        executeQuery(query);
    }


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

/**
 * @brief Handle the schedule of a drone. \n
 * It sends the path to the drone and waits for the next send. \n
 * It also updates the database with the drone's information.
 * @param schedule DroneSchedule that contains the path, the path id and the next send time.
 * @param ctx redisContext used to send the path to the drone.
 */
void ControlCenter::handleSchedule(DroneSchedule schedule, redisContext *ctx) {
    int pathId = get<0>(schedule);
    Path path = get<1>(schedule);
    chrono::milliseconds nextSend = get<2>(schedule);

    string pathStr = path.toString();

    string query = "INSERT INTO path (id, path) "
                   "VALUES (" + to_string(pathId) + ", '" + pathStr + "') "
                                                                      "ON CONFLICT (id) "
                                                                      "DO UPDATE SET path = EXCLUDED.path;";
    executeQuery(query);



    Redis::Message message;
    message["command"] = "follow-path";
    message["path"] = pathStr;

    string stream = "stream_drone_-1";

    // Send path to a ready drone every nextSend milliseconds
    while (!interrupt_.load()) {
        DroneData droneData = removeDroneFromReady();
        if (droneData.id == -1) {
            cerr << "ControlCenter::handleSchedule: Error: No drone available" << endl;
            break;
//          TODO: Handle better this case
        }

        stream = "stream_drone_" + to_string(droneData.id);

        // Send the path to the drone
        // cout << "[1]ControlCenter::sendPath: Sending path to drone "<< droneData.id << endl;
        Redis::sendMessage(ctx, stream, message);
        // cout << "[2]ControlCenter::sendPath: Sending path to drone " << droneData.id << endl;

        addDroneToWorking(droneData);

        query = "INSERT INTO drone_log (drone_id, cc_id, time, path_id, status) VALUES (" +
                       to_string(droneData.id) + ", " +
                       to_string(id_) + ", " +
                       "NOW(), " +
                       to_string(pathId) + ", " +
                       "'WORKING');";
        executeQuery(query);


        // Wait for the next send
        for (int i = 0; i < nextSend.count()/1000; i++) {
            if (interrupt_.load()) {
                break;
            }
            this_thread::sleep_for(chrono::seconds(1));
        }

//        this_thread::sleep_for(nextSend);
    }



    // Free the redis context
    redisFree(ctx);

}

/**
 * @brief Listen for the drones. \n
 * Makes a request to the redis server to read the messages from his group. \n
 * Than for each message it processes it and deletes it from the stream.
 */
void ControlCenter::listenDrones() {
    const string stream = "cc_" + to_string(id_);
    const string group = "CC_" + to_string(id_);
    const string consumer = "CC_" + to_string(id_);

    cout << "ControlCenter::listenDrones: Listening for drones" << endl;

    while (!interrupt_.load()) {
        long length = Redis::getStreamLen(listener_ctx_, stream);

        // MONITOR
        if (length > 4000) {
            string query = "INSERT INTO monitor_failure (cc_id, failure, message, time) VALUES (" +
                           to_string(id_) + ", " +
                           "'CC_OVERLOAD', " +
                           "'cc Stream length is too long: " + to_string(length) + "', " +
                           "NOW());";
            executeQuery(query);
        }

        if (length > 20000) {
            cerr << "ControlCenter::listenDrones: CC OVERLOAD:Stream length is too long: " << length << endl;
        }

        vector<Redis::Response> responses = Redis::readGroupMessages(listener_ctx_, group, consumer, stream, 0, 0);

        for (const auto &response : responses) {

            string messageId = get<0>(response);

            if (messageId.empty()) {
                cerr << "ControlCenter::listenDrones: Error: Empty message" << endl;
                continue;
            }

            Redis::Message message = get<1>(response);

            if (message["command"] == "quit") {
                cout << "ControlCenter::listenDrones: Received quit message" << endl;
                stop();
                break;
            }

            processMessage(messageId, message);

            // Delete message from the stream
            long n_delete = Redis::deleteMessage(listener_ctx_, stream, messageId);
            if (n_delete == -1) {
                cerr << "ControlCenter::listenDrones: Error: Can't delete message" << endl;
            }

        }
    }
    cout  << "ControlCenter::listenDrones: Stopped listening for drones" << endl;
}

/**
 * @brief Process the message received from the drones. \n
 * It updates the database with the drone's information and the area. \n
 * It also updates the drone's status in the lists.
 * @param message Redis::Message that contains the information of the drone.
 */
void ControlCenter::processMessage(const string& messageId, Redis::Message message) {

    // Create a DroneData object from the message
    DroneData droneData = DroneData();
    droneData.id = stoi(message["id"]);
    droneData.x = stoi(message["x"]);
    droneData.y = stoi(message["y"]);
    droneData.battery = stof(message["battery"]);
    droneData.state = to_state(message["state"]);
    bool changedState = message["changedState"] == "true";

    string query;

    // MONITOR - DRONE_OUT_OF_BATTERY
    if (droneData.battery <= 0 && droneData.state != DroneState::CHARGING) {
        cerr << "ControlCenter::listenDrones: Error: Drone " << droneData.id << " has no battery" << endl;
        query = "INSERT INTO monitor_failure (cc_id, failure, message, time) VALUES (" +
                to_string(id_) + ", " +
                "'DRONE_OUT_OF_BATTERY', " +
                "'Drone ran out of battery at point (" + to_string(droneData.x) + ", " + to_string(droneData.y) + ")', " +
                "NOW());";
        executeQuery(query);
        return;
    }


    if (droneData.state == DroneState::WORKING) {
        // convert messageId in timestamp format: "1619430000000-0" -> get first part
        string timestamp = messageId.substr(0, messageId.find('-'));
        area_.updatePoint(droneData.x, droneData.y, stoll(timestamp));


    }

    // aggiorna le informazioni del drone
    query = "UPDATE drone SET battery = " + to_string(droneData.battery) + ", " +
            "status = '" + to_string(droneData.state) + "' " +
            "WHERE id = " + to_string(droneData.id) + ";";
    executeQuery(query);


    if (!changedState) {
        return;
    }
    // else changedState == true

    // insertDroneLog(droneData);
    query = "INSERT INTO drone_log (drone_id, cc_id, time, path_id, status) VALUES (" +
            to_string(droneData.id) + ", " +
            to_string(id_) + ", " +
            "NOW(), " +
            "-1, ";

    switch (droneData.state) {
        case DroneState::READY:
            if (removeDroneFromCharging(droneData)) {
                addDroneToReady(droneData);
            }
            query += "'READY');";
            executeQuery(query);

            break;
        case DroneState::WORKING:
            // when change to WORKING, it's handled by handleSchedule thread
            break;
        case DroneState::CHARGING:
            if (removeDroneFromWorking(droneData)) {
                addDroneToCharging(droneData);
            }
            query += "'CHARGING');";
            executeQuery(query);
            break;
        default:
            cerr << "ControlCenter::listenDrones: Error: Invalid state" << endl;
            break;
    }

}

/* ------ Area ------ */
/**
 * @brief Sends area to server. \n
 * It sends the area status to the server every second and print the status of the area.
 */
void ControlCenter::sendAreaToServer() {
    while (!interrupt_.load()){
        bool area_covered = false;

        Grid grid = area_.getGrid();

        json jsonData;

        json areaJson;
        for (const auto &row : grid) {
            json rowJson;
            for (const auto &timestamp : row) {
                rowJson.push_back(timestamp);
            }
            areaJson.push_back(rowJson);
        }

        float area_percentage = area_.getPercentage();

        if (area_percentage >= 1) {
            area_covered = true;
        }

        // MONITOR
        if (area_covered && area_percentage < 1) {
            cerr << "ControlCenter::sendAreaToServer: Error: Area covered but percentage < 1" << endl;
            string query = "INSERT INTO monitor_failure (cc_id, failure, message, time) VALUES (" +
                            to_string(id_) + ", " +
                            "'AREA_COVERAGE', " +
                            "'Area covered but percentage now < 1', " +
                            "NOW());";
            executeQuery(query);
        }

        jsonData["area"] = areaJson;
        jsonData["cc-id"] = id_;
        jsonData["area_percentage"] = area_percentage;

        cout << "Sending data to server" << endl;
        // curl to server
        string url = "http://localhost:3000/report";
        string data = jsonData.dump();

        curl_global_init(CURL_GLOBAL_ALL);
        CURL *curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            curl_easy_setopt(curl, CURLOPT_POST, 1L);

            // Impostazione del tipo di contenuto della richiesta
            struct curl_slist *headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/json");

            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            // Set the POST data
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());

            // Perform the request
            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
            }

            // Clean up
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
        }

        cout << "Sent data to server" << endl;


        cout << "Cover percentage of Area: " << area_percentage * 100 << "%" << endl;
        // wait 10 seconds
        this_thread::sleep_for(chrono::seconds(1));
    }
}




/* ------ DB ------ */
/**
 * @brief Execute the query in the database. \n
 * It locks the query_mutex_ before executing the query.
 * @param query string that contains the query to execute.
 */
void ControlCenter::executeQuery(const std::string &query) {
    std::lock_guard<std::mutex> lock(query_mutex_);
    conn_.ExecSQLcmd(const_cast<char *>(query.c_str()));
}


/* ------ List Mutex ------ */
/**
 * @brief Add a drone to the working drones list. \n
 * It locks the lists_mutex_ before adding the drone to the list.
 * @param drone DroneData that represents the drone to add.
 */
void ControlCenter::addDroneToWorking(const DroneData &drone) {
    std::lock_guard<std::mutex> lock(lists_mutex_);
    workingDrones_.push_back(drone);
}

/**
 * @brief Add a drone to the charging drones list. \n
 * It locks the lists_mutex_ before adding the drone to the list.
 * @param drone DroneData that represents the drone to add.
 */
void ControlCenter::addDroneToCharging(const DroneData &drone) {
    std::lock_guard<std::mutex> lock(lists_mutex_);
    chargingDrones_.push_back(drone);
}

/**
 * @brief Add a drone to the ready drones list. \n
 * It locks the lists_mutex_ before adding the drone to the list.
 * @param drone DroneData that represents the drone to add.
 */
void ControlCenter::addDroneToReady(const DroneData &drone) {
    std::lock_guard<std::mutex> lock(lists_mutex_);
    readyDrones_.push_back(drone);
}

/**
 * @brief Remove a drone from the working drones list. \n
 * It locks the lists_mutex_ before removing the drone from the list.
 * @param droneToRemove DroneData that represents the drone to remove.
 * @return true if the drone is removed, false otherwise.
 */
bool ControlCenter::removeDroneFromWorking(const DroneData &droneToRemove) {
    std::lock_guard<std::mutex> lock(lists_mutex_);
    auto it =
            std::find(workingDrones_.begin(), workingDrones_.end(), droneToRemove);
    if (it != workingDrones_.end()) {
        workingDrones_.erase(it);
        return true;
    }
    return false;
}

/**
 * @brief Remove a drone from the charging drones list. \n
 * It locks the lists_mutex_ before removing the drone from the list.
 * @param droneToRemove DroneData that represents the drone to remove.
 * @return true if the drone is removed, false otherwise.
 */
bool ControlCenter::removeDroneFromCharging(const DroneData &droneToRemove) {
    std::lock_guard<std::mutex> lock(lists_mutex_);
    auto it = std::find(chargingDrones_.begin(), chargingDrones_.end(),
                        droneToRemove);
    if (it != chargingDrones_.end()) {
        chargingDrones_.erase(it);
        return true;
    }
    return false;
}

/**
 * @brief Remove a drone from the ready drones list. \n
 * It locks the lists_mutex_ before removing the drone from the list.
 * @return DroneData that represents the drone removed.
 */
DroneData ControlCenter::removeDroneFromReady() {
    std::lock_guard<std::mutex> lock(lists_mutex_);
    if (readyDrones_.empty()) {
        // Se la lista è vuota, restituisci un drone vuoto
        return DroneData{};
    }
    DroneData drone = readyDrones_.front();
    readyDrones_.pop_front();
    return drone;
}










