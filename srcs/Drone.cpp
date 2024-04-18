#include "Drone.hpp"
#include <string>
#include <iostream>

// Costruttore
Drone::Drone(int id, int cc_id) : id_(id), cc_id_(cc_id), redisClient_("localhost", 6379){
    std::string command = "XADD control_centers * message drone_" + std::to_string(id_) + "_connected";
    redisClient_.sendCommand(command);
    std::cout << "Drone " << id_ << " connesso al ControlCenter " << cc_id_ << std::endl;
    RedisClient redisClient = RedisClient("localhost", 6379);
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
    redisClient_.sendCommand("PUBLISH control_center path_request_from_drone_" + std::to_string(id_));
    
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
