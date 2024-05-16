//
// Created by Gianluca Viviano on 11/05/24.
//

#ifndef DRONE8_BASICSTRATEGY_H
#define DRONE8_BASICSTRATEGY_H

#include "ScanningStrategy.h"

class BasicStrategy {
public:
    BasicStrategy() {
        std::cout << "BasicStrategy::BasicStrategy: starting" << std::endl;
    }
    vector<DroneSchedule> createSchedules(Area area);
    tuple<Coordinate, int> getAvailableCoord(int autonomy, Coordinate current_position, Coordinate next_position, Coordinate cc_pos);
    tuple<Coordinate, int> goSouth(int autonomy, Coordinate current_position, Coordinate end, Coordinate cc_pos);
    Path returnToCC(int autonomy, Coordinate current_position, Coordinate cc_pos);

    int manhattanDistance(Coordinate a, Coordinate b);
};

#endif //DRONE8_BASICSTRATEGY_H
