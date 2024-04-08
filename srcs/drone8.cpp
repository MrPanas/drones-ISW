#include <iostream>
#include <string>
#include "Drone.hpp"
#include "ControlCenter.hpp"

int main() {
    std::cout << "Drone e ControlCenter avviati correttamente." << std::endl;
    // Creazione del drone e del control center
    Drone drone(1, 1);
    ControlCenter controlCenter(1);

    return 0;
}