#include <iostream>
#include <string>
#include <hiredis/hiredis.h>
#include "Drone.hpp"
#include "ControlCenter.hpp"

int main() {
    std::cout << "Test avviato correttamente." << std::endl;
    // Creazione del drone e del control center
    
    ControlCenter controlCenter(1);
    Drone drone(1, 1);
    

    controlCenter.powerOn();
    // drone.powerOn();

    drone.requestPath(); // TODO: fare in modo che sia l'algoritmo a decidere ogni quanto chiamare questa funzione (modularità)

    return 0;
}