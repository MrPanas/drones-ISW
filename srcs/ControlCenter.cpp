#include "ControlCenter.hpp"
#include <iostream>
#include <thread>

using namespace std;

ControlCenter::ControlCenter(int id) : id_(id), redisClient_("localhost", 6379) {
    // redisClient_.sendCommand("SUBSCRIBE control_center");
    // redisClient_.sendCommand("XGROUP CREATE control_centers mygroup $ MKSTREAM")
}

// Metodo per gestire la ricezione delle richieste dal drone sul canale control_center
void ControlCenter::handleDroneRequests() {
    // Avviare un nuovo thread per gestire la ricezione dei messaggi
    std::thread receiverThread([this]() {
        while (true) {
            if (this->redisClient_.hasReply()) {
                std::cout << "Ricevuto messaggio" << std::endl;
                std::string command = "XREAD STREAMS control_centers 0";

                redisClient_.sendCommand(command);
                // Dentro la lambda function
                redisClient_.getReply();

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


