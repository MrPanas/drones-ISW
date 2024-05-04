//
// Created by Patryk Mulica    on 30/04/24.
//

#ifndef DRONE8_SCANNINGSTRATEGY_H
#define DRONE8_SCANNINGSTRATEGY_H

#include "../area/Area.hpp"
#include "../drone/Drone.h"

using namespace std;

/**
 * Enum for the directions
 */
enum Direction {
    NORTH,
    SOUTH,
    WEST,
    EAST
};

/**
 * Convert a direction to a string
 * @param direction
 * @return direction as string
 */
inline string toString(Direction direction) {
    switch (direction) {
        case NORTH:
            return "N";
        case SOUTH:
            return "S";
        case WEST:
<<<<<<< HEAD
            return "W";
        case EST:
            return "E";
=======
            return "WEST";
        case EAST:
            return "EAST";
>>>>>>> e09dea4 (test3)
        default:
            return "UNKNOWN";
    }
}
struct DroneData;

/**
 * Path class
 */
class Path {
    using DirectionList = vector<tuple<Direction, int>>;
public:
    Path(DirectionList path);
    Path();

    DirectionList getPath() const;

    void addDirection(Direction dir, int steps);

private:
    DirectionList path_;
};



using DroneSchedule = tuple<int, Path, chrono::milliseconds>;

/**
 * Interface for the scanning strategy
 */
class ScanningStrategy {
public:
    virtual ~ScanningStrategy() = default; // Distruttore virtuale per la classe astratta
    virtual vector<DroneSchedule> createSchedules(Area area);
};


#endif //DRONE8_SCANNINGSTRATEGY_H
