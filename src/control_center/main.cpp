//
// Created by Patryk Mulica    on 02/05/24.
//

#include "ControlCenter.hpp"
#include "../drone/Drone.h"
#include "../scanning_strategy/BasicStrategy.h"
#include "../area/Area.hpp"


#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {

    cout << "Hello, World!" << endl;
    /*
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
     */

    Area area = Area(100, 100);

    BasicStrategy strategy = BasicStrategy();

    vector<DroneSchedule> schedules = strategy.createSchedules(area);

    cout << "Schedules calculated" << endl;

    cout << "Schedules length: " << schedules.size() << endl;

    cout << "path legth " << get<1>(schedules[0]).getPath().size() << endl;

    cout << "path: " << get<1>(schedules[0]).toString() << endl;

    //for (DroneSchedule schedule : schedules) {
    //    cout << "Drone: " << get<0>(schedule) << " Path: " << get<1>(schedule).toString() << " Time: " << get<2>(schedule).count() << endl;
    //}





    return EXIT_SUCCESS;
}
