//
// Created by Patryk Mulica    on 02/05/24.
//

#include "ControlCenter.hpp"
#include "../drone/Drone.h"


#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {

    cout << "Hello, World!" << endl;

    ControlCenter controlCenter = ControlCenter(1, 1);
    Drone drone1 = Drone(1, 1);
    Drone drone2 = Drone(2, 1);
    Drone drone3 = Drone(3, 1);
    Drone drone4 = Drone(4, 1);


    thread drone_thread(&Drone::start, &drone1);
    thread drone_thread2(&Drone::start, &drone2);
    thread drone_thread3(&Drone::start, &drone3);
    thread drone_thread4(&Drone::start, &drone4);
    cout << "Drone thread started" << endl;
    thread cc_thread(&ControlCenter::start, &controlCenter);

    // Attende che entrambi i thread terminino
    drone_thread.join();
    drone_thread2.join();
    drone_thread3.join();
    drone_thread4.join();
    cc_thread.join();

    return EXIT_SUCCESS;
}
