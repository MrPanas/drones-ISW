//
// Created by Patryk Mulica    on 30/04/24.
//

#ifndef DRONE8_CONTROLCENTER_HPP
#define DRONE8_CONTROLCENTER_HPP

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <hiredis/hiredis.h>
#include <atomic>
#include "../scanning_strategy/ScanningStrategy.h"
#include "../area/Area.hpp"
#include "../drone/Drone.h"
#include "../con2db/pgsql.h"

enum class ControlCenterState {
    STARTING,
    SENDING_PATHS,
    WAITING,
    QUITTING
}; // TODO vedere se serve a qualcosa


using namespace std;


class ControlCenter {
public:
    ControlCenter(int id);


    ControlCenter(int id, ScanningStrategy *strategy, Area area);

    void setStrategy(ScanningStrategy *strategy);

    void start();

    void stop();

    // Destructor
    ~ControlCenter();

private:

    void listenDrones();

    void sendPathsToDrones();

    void sendPath(int droneId, Path path);

    void insertLog();

    const int id_;
    ScanningStrategy* strategy_;
    Area area_;
    redisContext *ctx_;
    vector<DroneData> drones;
    PGconn *conn_;
};

//privte:
//    std::vector<std::string> drones = {"drone1", "drone2", "drone3"};
//    ControlCenterState state;
//    bool stopFlag;
//    // timeStamp of start
//    std::chrono::time_point<std::chrono::system_clock> start_time;
//    // timeStamp of end
//    std::chrono::time_point<std::chrono::system_clock> end_time;
//
//
//    // Metodo per inviare il percorso a un drone specifico
//    void sendPathToDrone(int droneId) {
//        printState();
//        // Simulazione: Invia il percorso al drone con ID droneId
//        std::cout << "Sending path to drone " << drones[droneId] << std::endl;
//        std::this_thread::sleep_for(std::chrono::seconds(1)); // Simulazione di invio
//    }
//
//    // Metodo per ricevere i dati dai droni
//    void receiveDataFromDrones() {
//        printState();
//        // Simulazione: Ricevi i dati dai droni utilizzando Redis Stream
//        std::cout << "Receiving data from drones..." << std::endl;
//        std::this_thread::sleep_for(std::chrono::seconds(2)); // Simulazione di ricezione
//    }
//
//    // Metodo per stampare lo stato del controllo
//    void printState() {
//        std::string stateString;
//        switch (state) {
//            case ControlCenterState::STARTING:
//                stateString = "STARTING";
//                break;
//            case ControlCenterState::SENDING_PATHS:
//                stateString = "SENDING_PATHS";
//                break;
//            case ControlCenterState::WAITING:
//                stateString = "WAITING";
//                break;
//            case ControlCenterState::QUITTING:
//                stateString = "QUITTING";
//                break;
//            default:
//                stateString = "UNKNOWN";
//        }
//        std::cout << "Control Center State: " << stateString << std::endl;
//    }
//    void isTimeToDie(){
//        // if now() > end_time
//        if (std::chrono::system_clock::now() > end_time) {
//            stopFlag = true;
//        }
//    }
//
//public:
//    ControlCenter() : state(ControlCenterState::STARTING), stopFlag(false) {
//        start_time = std::chrono::system_clock::now();
//        // end_time = now() + 10 seconds
//        end_time = start_time + std::chrono::seconds(10);
//    }
//
//    void startOperations() {
//        isTimeToDie();
//
//        if (stopFlag) {
//            return;
//        }
//
//        state = ControlCenterState::SENDING_PATHS;
//
//        // Open n threads for each drone
//        // send path to each drone on stream that is a group
//        // and wait for ack
//
//        std::vector<std::thread> sendThreads;
//        sendThreads.reserve(drones.size());
//        for (size_t i = 0; i < drones.size(); ++i) {
//            sendThreads.emplace_back(&ControlCenter::sendPathToDrone, this, i);
//        }
//
//        // Attendi che tutti i thread di invio terminino
//        for (auto& thread : sendThreads) {
//            thread.join();
//        }
//
//        state = ControlCenterState::WAITING;
//
//        // Avvia il thread per ricevere i dati dai droni
//        std::thread receiveDataThread(&ControlCenter::receiveDataFromDrones, this);
//
//        // Attendi che il thread di ricezione termini
//        receiveDataThread.join();
//
//        startOperations();
//    }
//
//
//};
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
////#include <vector>
////#include <thread>
//#include <vector>
//#include <mutex>
//#include <condition_variable>
//#include <atomic>
//#include "../scanning_strategy/ScanningStrategy.h"
//#include "../drone/Drone.h"
//#include "../redis/StreamRedis.h"
//
//// Possible states of the ControlCenter
//enum class ControlCenterState {
//    STARTING,
//    SENDING_PATHS,
//    WAITING,
//    QUITTING
//};
//
//class ControlCenter {
//public:
//    // Costruttore
//    ControlCenter(int id, ScanningStrategy* strategy);
//
//
//    // Metodo per aggiungere un drone al centro di controllo
//    void addDrone(const std::string& drone_stream);
//
//    // Metodo per eseguire la scansione dell'area
//    void executeScan() const;
//
//    // Metodo per impostare la strategia di scansione
//    void setStrategy(ScanningStrategy* strategy);
//
//
//
//    // Distruttore
//    ~ControlCenter() {
//        stopOperations();
//    }
//
//    // Metodo per avviare le operazioni del Centro di Controllo
//    void startOperations() {
//        // Avvio del thread per l'ascolto dei droni
//        listenThread = std::thread(&ControlCenter::listenToDrones, this);
//
//        // Avvio dei thread per l'invio dei path ai droni
//        for (int i = 0; i < 10; ++i) {
//            sendThreads.emplace_back(&ControlCenter::sendPathsToDrone, this, i);
//        }
//    }
//
//    // Metodo per interrompere le operazioni del Centro di Controllo
//    void stopOperations() {
//        // Interrompi tutti i thread
//        stopFlag = true;
//
//        // Attendi che tutti i thread terminino
//        if (listenThread.joinable()) {
//            listenThread.join();
//        }
//        for (auto& thread : sendThreads) {
//            if (thread.joinable()) {
//                thread.join();
//            }
//        }
//    }
//
//
//
//private:
//    int id_; // ID del centro di controllo
//    // Vector of string that represent the name of stream with drones
//    std::vector<std::string> drone_streams_; // e.g. "path_drone1", "path_drone2", "path_drone3"
//    ScanningStrategy* strategy_; // Strategia di scansione
//    // StreamRedis
//    StreamRedis* stream_redis_;
//
//
//    std::mutex mutex;
//    ControlCenterState state;
//    std::vector<std::string> paths = {"path1", "path2", "path3"};
//    std::condition_variable termination;
//
//
//    std::thread listenThread; // Thread per l'ascolto dei droni
//    std::vector<std::thread> sendThreads; // Thread per l'invio dei path ai droni
//    bool stopFlag; // Flag per interrompere i thread
//
//    // Funzione che ascolta cosa dicono i droni
//    void listenToDrones() {
//        while (!stopFlag) {
//            // Codice per ascoltare i droni
//            std::this_thread::sleep_for(std::chrono::seconds(1)); // Simulazione
//            std::cout << "Listening to drones..." << std::endl;
//        }
//    }
//
//    // Funzione per inviare i path ai droni
//    void sendPathsToDrone(int droneId) {
//        while (!stopFlag) {
//            // Codice per inviare i path al drone con ID droneId
//            std::this_thread::sleep_for(std::chrono::seconds(1)); // Simulazione
//            std::cout << "Sending path to drone " << droneId << std::endl;
//        }
//    }
//
//
//
//};



#endif //DRONE8_CONTROLCENTER_HPP
