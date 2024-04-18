#include "ControlCenter.hpp"
#include <iostream>
#include <thread>

using namespace std;

ControlCenter::ControlCenter(int id) : id_(id), redisClient_("localhost", 6379) {
    // redisClient_.sendCommand("SUBSCRIBE control_center");
}

// Metodo per gestire la ricezione delle richieste dal drone sul canale control_center
void ControlCenter::handleDroneRequests() {
    // Avviare un nuovo thread per gestire la ricezione dei messaggi
    std::thread receiverThread([this]() {
        while (true) {
            if (this->redisClient_.hasReply()) {
                std::string command = "XREAD STREAMS control_centers 0";
                redisClient_.sendCommand(command);
                std::string message = redisClient_.getReply();
                // std::string message = redisClient_.getReply();
                std::cout << "Messaggio ricevuto: " << message << std::endl;

                // Elaborare il messaggio ricevuto
                // Ad esempio, se il messaggio contiene una richiesta di percorso dal drone
                // il Control Center può elaborare la richiesta e inviare una risposta al drone
                if (message.find("drone_1_connected") != std::string::npos) {
                    std::cout << "Bella per il drone 1" << std::endl;
                    // Elaborare la richiesta e inviare una risposta al drone
                    // Ad esempio:
                    // std::vector<std::tuple<Direction, int>> path = computePath();
                    // sendPathToDrone(path);
                }
                if (message.find("path_request_from_drone_1") != std::string::npos) {
                    std::cout << "Richiesta di percorso ricevuta dal drone 1" << std::endl;
                    // Elaborare la richiesta e inviare una risposta al drone
                    // Ad esempio:
                    // std::vector<std::tuple<Direction, int>> path = computePath();
                    // sendPathToDrone(path);
                }
            }
        }
    });
    // Attendere la terminazione del thread ricevitore prima di uscire dal metodo
    receiverThread.join();
}

void ControlCenter::sendPath(int droneId, std::string& path) {
    // Invia un messaggio al drone sul canale specifico
    std::string channel = "drone_" + std::to_string(droneId);
    redisClient_.sendCommand("PUBLISH " + channel + " '" + path + "'");
}


void ControlCenter::powerOn() {
    // Avvia il thread per gestire le richieste dei droni
    handleDroneRequests();
}

void ControlCenter::powerOff() {
    // Disconnettersi dal server Redis
    // redisClient_.disconnect(); // ???
}

void ControlCenter::setAlgorithm(Algorithm* alg) {
    algorithm = alg;
}


