//
// Created by Patryk Mulica    on 02/05/24.
//

#include <iostream>
#include <vector>
#include "../redis/StreamRedis.h"
#include "../drone/Drone.h"
#include "../control_center/ControlCenter.hpp"

using namespace std;



int main(int argc, char *argv[]) {
    cout << "Hello, World!" << endl;

    // ControlCenter controlCenter = ControlCenter(1);

    Drone drone = Drone(1, 1);

    drone.start();
    // controlCenter.start();

    return 0;
}