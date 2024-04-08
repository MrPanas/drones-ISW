#ifndef DRONE_HPP
#define DRONE_HPP

#include <iostream>
#include <string>
#include "ControlCenter.hpp"

class Drone {
public:
    // Costruttore
    Drone(int id, ControlCenter* cc);

    // Metodo per ottenere l'ID del drone
    int getId() const;

    std::vector<std::tuple<Direction, int>> requestPath();

    void followPath();

private:
    int id_;
    ControlCenter* controlCenter;
};

#endif
