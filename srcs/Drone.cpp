#include "Drone.hpp"

// Costruttore
Drone::Drone(int id, ControlCenter* cc) : id_(id) {
    // Inizializza il control center del drone
    controlCenter = cc;
}

// Metodo per ottenere l'ID del drone
int Drone::getId() const {
    return id_;
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
