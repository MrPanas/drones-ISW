//
// Created by Patryk Mulica    on 02/05/24.
//

#include <iostream>
#include <vector>
#include "../redis/StreamRedis.h"
#include "../drone/Drone.h"

using namespace std;



int main(int argc, char *argv[]) {
    /*
    string id = argv[1];

    cout << "Hello I'm a Drone" << id << "!!" << endl;

    // create StreamRedis object
    StreamRedis streamRedis;
    string myStream = "drone" + id;

    // Waiting till the ControlCenter sends the path
     */

    cout << "Hello, World!" << endl;

    Drone drone = Drone(1);

    drone.start();






    return 0;
}