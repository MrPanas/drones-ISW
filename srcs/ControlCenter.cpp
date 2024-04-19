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
    std::cout << "ControlCenter " << id_ << " in ascolto delle richieste dei droni" << std::endl;
    while (true) {
        std::string command = "XRANGE control_centers - +";

        // Invia il comando al server Redis
        redisReply *reply = (redisReply *)redisCommand(this->ctx_, command.c_str());
        // std::cout << "Comando inviato" << std::endl;
        if (reply == NULL) {
            std::cerr << "Errore nell'invio del comando XRANGE" << std::endl;
            continue;
        }

        if (reply->type == REDIS_REPLY_ARRAY && reply->elements > 0) {
            std::cout << "Ricevuto messaggio" << std::endl;
            // print reply type
            // get last element
            redisReply *message = reply->element[0];
            if (reply == NULL) {
                std::cout << "Errore nell'invio del comando XREAD" << std::endl;
                continue;
            }
            
            json jsonObject = redisReplyToJSON(message);

            std::string id = jsonObject["id"];

            std::cout << "JSON: " << jsonObject << std::endl;

            deleteMessage("control_centers", id, this->ctx_);

            // TODO: al posto di fare tanti if con contains per vedere cosa vuole, si può fare che il primo field 
            //      del messaggio sia il comando che si vuole fare in modo che una volta individuato abbia una struttura precisa
            if (jsonObject["data"].contains("request_path")) { 
                std::cout << "Richiesta di percorso ricevuta" << std::endl;
                // Calcola il percorso da inviare al drone
                std::string drone_id = jsonObject["data"]["request_path"];
                // std::string path = algorithm->computePath();
                std::string path = "path_calculated";

                // Invia il percorso al drone
                redisReply *reply = (redisReply *)redisCommand(this->ctx_, "XADD %s * %s %s", drone_id.c_str(), "path", path.c_str());
                if (reply == NULL) {
                    std::cout << "Errore nell'invio del comando XADD" << std::endl;
                } else {
                    std::cout << "Comando XADD inviato" << std::endl;
                    freeReplyObject(reply);
                }
            }
        }
    }
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


