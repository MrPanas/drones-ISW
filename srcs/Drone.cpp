#include "Drone.hpp"
#include <string>
#include <iostream>
#include "redis_fnc.hpp"
#include <thread>

// Costruttore
Drone::Drone(int id, int cc_id) : id_(id), cc_id_(cc_id){
    ctx_ = redisConnect("127.0.0.1", 6379);
    if (ctx_ == NULL || ctx_->err) {
        if (ctx_) {
            printf("Errore: %s\n", ctx_->errstr);
            redisFree(ctx_);
        } else {
            printf("Errore: impossibile allocare redis context\n");
        }
        exit(1);
    }
    std::cout << "connessione a redis riuscita" << std::endl;

    // Creare un gruppo con l'id del drone
    createGroup(ctx_, "drone_" + std::to_string(id_), "mygroup");


    // send XADD command
    // DEBUG____
    // redisReply *reply = (redisReply *)redisCommand(ctx_, "XADD %s * %s %s %s %s", "control_centers", "field1", "value1", "field2", "value2");
    // if (reply == NULL) {
    //     std::cout << "Errore nell'invio del comando XADD" << std::endl;
    // } else {
    //     std::cout << "Comando XADD inviato" << std::endl;
    //     freeReplyObject(reply);
    // }
    // _______
}

void Drone::handleCcRequests() {
    std::cout << "Drone " << id_ << " in ascolto delle richieste del ControlCenter" << std::endl;
    while (true) {
        std::string command = "XRANGE drone_" + std::to_string(id_) + " - +";

        // Invia il comando al server Redis
        redisReply *reply = (redisReply *)redisCommand(this->ctx_, command.c_str());
        if (reply == NULL) {
            std::cerr << "Errore nell'invio del comando XRANGE" << std::endl;
            return;
        }

        if (reply->type == REDIS_REPLY_ARRAY && reply->elements > 0) {
            std::cout << "drone_" << id_ << " Ricevuto messaggio" << std::endl;
            redisReply *message = reply->element[0];
            if (reply == NULL) {
                std::cout << "Errore nell'invio del comando XREAD" << std::endl;
                return;
            }
            

            json jsonObject = redisReplyToJSON(message);

            std::string id = jsonObject["id"];

            std::cout << "JSON: " << jsonObject << std::endl;

            deleteMessage("drone_" + std::to_string(id_), id, this->ctx_);

            // TODO stesso discorso del control center
            if (jsonObject["data"].contains("path")) {
                std::cout << "Percorso ricevuto" << std::endl;
                // Calcola il percorso da inviare al drone
                std::string path = jsonObject["data"]["path"];
                // std::string path = algorithm->computePath();
                std::cout << "Percorso ricevuto: " << path << std::endl;
            }
        }
    }
}


void Drone::powerOn() {
    // Creare un gruppo con l'id del drone e fare tutto quello nel costruttore qua?
    handleCcRequests();
}


// Metodo per ottenere l'ID del drone
int Drone::getId() const {
    return id_;
}

int Drone::getControlCenterId() const {
    return cc_id_;
}

// Metodo per richiedere un percorso al ControlCenter
std::vector<std::tuple<Direction, int>> Drone::requestPath() {
    std::cout << "Richiesta percorso al ControlCenter" << std::endl;
    // Chiedi al ControlCenter il percorso da seguire
    // return controlCenter->computePath();
    std::string command = "XADD control_centers * request_path drone_" + std::to_string(id_);
    redisReply *reply = (redisReply *)redisCommand(ctx_, command.c_str());
    std::cout << "Richiesta inviata" << std::endl;
    if (reply == NULL) {
        std::cout << "Errore nell'invio del comando XADD" << std::endl;
    } else {
        std::cout << "Comando XADD inviato" << std::endl;
        freeReplyObject(reply);
    }
    return std::vector<std::tuple<Direction, int>>(); 
}

// Metodo per seguire un percorso
void Drone::followPath() {
    // Richiedi un percorso al ControlCenter
    std::vector<std::tuple<Direction, int>> path = requestPath();

    // Segui il percorso
    for (const auto& step : path) {
        Direction direction = std::get<0>(step);
        int distance = std::get<1>(step);

        // Esegui l'azione necessaria per muoversi nella direzione specificata
        // ad esempio, volare o spostarsi a terra
        std::cout << "Muoviti " << distance << " metri nella direzione " << direction << std::endl;
    }
}
