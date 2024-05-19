

#ifndef DRONE8_BASICSTRATEGY_H
#define DRONE8_BASICSTRATEGY_H

#include "ScanningStrategy.h"
#include "../config.h"

class BasicStrategy : public ScanningStrategy {
public:
    BasicStrategy();
    ~BasicStrategy();
    vector<DroneSchedule> createSchedules(Area area) override;
    tuple<Coordinate, bool> goToPoint(int autonomy, Coordinate current_position, Coordinate next_position, Coordinate cc_pos, Path& path, bool comeBack);

    //int manhattanDistance(Coordinate a, Coordinate b);
};

#endif //DRONE8_BASICSTRATEGY_H
