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

    // initialize grid 300*300
    vector<vector<int>> grid(300, vector<int>(300, 0));
    int cc_x = 150;
    int cc_y = 150;
    grid[cc_x][cc_y] = 1;
    // che if all the paths cover all the area
    for (DroneSchedule schedule : schedules) {
        Path pathP = get<1>(schedule);
        string path = pathP.toString();
        size_t i = 0;
        while (i < path.length()) {
            int x = 150;
            int y = 150;
            char dir = path[i++];
            string stepsStr;
            while (i < path.length() && isdigit(path[i])) {
                stepsStr += path[i++];
            } // format is <dir><steps>_<dir><steps>... this reads the direction and the steps

            i++;
            int steps = stoi(stepsStr);

            for (int j = 0; j < steps; j++) {
                switch (dir) {
                    case 'N':
                        y--;
                        break;
                    case 'S':
                        y++;
                        break;
                    case 'E':
                        x--;
                        break;
                    case 'W':
                        x++;
                        break;
                    default:
                        cerr << "Drone::followPath: Invalid direction" << endl;
                }
                grid[x][y] = 1;
            }
        }
        // Print if autonomy != 0
    }

    // Print grid
    for (int i = 0; i < 300; i++) {
        for (int j = 0; j < 300; j++) {
            if (grid[i][j] == 0) {
                cout << "x: " << i << " y: " << j << endl;
                continue;
            }
        }
        cout << endl;
    }


    // Calcolo numero di droni necessari
    // 3h / 4.50min = 36
    // 36 * numero di droni = 8600





    return EXIT_SUCCESS;
}
