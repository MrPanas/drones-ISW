#include <iostream>
#include <string>
#include <hiredis/hiredis.h>
#include "Drone.hpp"
#include "ControlCenter.hpp"
#include <thread>


int main() {

    std::cout << "Test avviato correttamente." << std::endl;
    // Creazione del drone e del control center
    
    ControlCenter controlCenter(1);
    Drone drone(1, 1);
    
    std::thread ccThread([&controlCenter]() {
        controlCenter.powerOn();
    });
    std::thread droneThread([&drone]() {
        drone.powerOn();
    });
    // controlCenter.powerOn();
    // drone.powerOn();

    std::cout << "Thread superato" << std::endl;

    drone.requestPath(); // TODO: fare in modo che sia l'algoritmo a decidere ogni quanto chiamare questa funzione (modularità)

    for (int i = 0; i < 100; i++) {
        std::cout << "Main thread" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    ccThread.join();
    droneThread.join();

    return 0;
}