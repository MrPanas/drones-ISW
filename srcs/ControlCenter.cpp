#include <iostream>
#include <thread>
#include "redis_fnc.hpp"
#include "ControlCenter.hpp"

ControlCenter::ControlCenter(int id) : id_(id), ctx_(redisConnect("127.0.0.1", 6379)) {

    // Struttura redis:
    // STREAM control_center per ricevere richieste dai droni
    // STREAM drone_<id> per inviare messaggi ai droni

    // redisClient_.sendCommand("SUBSCRIBE control_center");
    createGroup(ctx_, "control_centers", "mygroup");
}

// Metodo per gestire la ricezione delle richieste dal drone sul canale control_center
void ControlCenter::handleDroneRequests() {
    // Avviare un nuovo thread per gestire la ricezione dei messaggi
    std::thread receiverThread([this]() {
        while (true) {
            std::string command = "XRANGE control_centers - +";
            // std::string command = "XREAD STREAMS control_centers 0";

            // Invia il comando al server Redis
            redisReply *reply = (redisReply *)redisCommand(this->ctx_, command.c_str());
            if (reply == NULL) {
                std::cerr << "Errore nell'invio del comando XRANGE" << std::endl;
                return;
            }

            if (reply->type == REDIS_REPLY_ARRAY && reply->elements > 0) {
                std::cout << "Ricevuto messaggio" << std::endl;
                // print reply type
                // get last element
                redisReply *message = reply->element[0];
                if (reply == NULL) {
                    std::cout << "Errore nell'invio del comando XREAD" << std::endl;
                    return;
                }
                

                json jsonObject = redisReplyToJSON(message);

                std::string id = jsonObject["id"];

                std::cout << "JSON: " << jsonObject << std::endl;
                std::cout << "value1: " << jsonObject["data"]["field1"] << std::endl;

                if (jsonObject["data"].contains("request_path")) {
                    // Calcola il percorso da inviare al drone
                    drone_id = jsonObject["data"]["request_path"];
                    // std::string path = algorithm->computePath();
                    std::string path = "path_calculated";
                    // Invia il percorso al drone
                    redisReply *reply = (redisReply *)redisCommand(this->ctx_, "XADD drone_%s * %s %s", "drone_" + drone_id, "path", path);
                    if (reply == NULL) {
                        std::cout << "Errore nell'invio del comando XADD" << std::endl;
                    } else {
                        std::cout << "Comando XADD inviato" << std::endl;
                        freeReplyObject(reply);
                    }


                    // Elimina il messaggio dalla coda
                    deleteMessage("control_centers", id, this->ctx_);
                }
                deleteMessage("control_centers", id, this->ctx_);
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


