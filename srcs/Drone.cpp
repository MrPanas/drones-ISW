#include "Drone.hpp"
#include <string>
#include <iostream>

// Costruttore
Drone::Drone(int id, int cc_id) : id_(id), cc_id_(cc_id), redisClient("localhost", 6379){
    channelName_ = "cc_" + std::to_string(cc_id);
    std::cout << "Drone " << id_ << " connesso al canale " << channelName_ << std::endl;
    if (redisClient.sendCommand("PUBLISH " + channelName_ + " Richiesta_del_drone")) {
        std::cout << "Comando inviato con successo a Redis." << std::endl;
    } else {
        std::cerr << "Errore nell'invio del comando a Redis." << std::endl;
    }
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
    // Chiedi al ControlCenter il percorso da seguire
    // return controlCenter->computePath();
    
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
