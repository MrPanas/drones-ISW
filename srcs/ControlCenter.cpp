#include "ControlCenter.hpp"
#include <iostream>
#include <thread>
#include "redis_fnc.hpp"


ControlCenter::ControlCenter(int id) : id_(id), ctx_(redisConnect("127.0.0.1", 6379)) {
    // redisClient_.sendCommand("SUBSCRIBE control_center");
    // redisClient_.sendCommand("XGROUP CREATE control_centers mygroup $ MKSTREAM")
}

// Metodo per gestire la ricezione delle richieste dal drone sul canale control_center
void ControlCenter::handleDroneRequests() {
    // Avviare un nuovo thread per gestire la ricezione dei messaggi
    std::thread receiverThread([this]() {
        while (true) {
            if (true) {
                std::cout << "Ricevuto messaggio" << std::endl;
                std::string command = "XRANGE control_centers - +";

                // Invia il comando al server Redis
                redisReply *reply = (redisReply *)redisCommand(this->ctx_, command.c_str());
                // print reply type
                // get last element
                redisReply *lastElement = reply->element[reply->elements - 1];
                std::cout << "Reply type: " << reply->type << std::endl;                
                std::cout << "Comando inviato: " << reply << std::endl;
                if (reply == NULL) {
                    std::cout << "Errore nell'invio del comando XREAD" << std::endl;
                    return;
                }
                

                json jsonObject = redisReplyToJSON(lastElement);


                std::cout << "JSON: " << jsonObject << std::endl;

                
            }
            break;
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


