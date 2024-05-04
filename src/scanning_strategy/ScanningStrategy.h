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
    EST
};

/**
 * Convert a direction to a string
 * @param direction
 * @return direction as string
 */
inline string toString(Direction direction) {
    switch (direction) {
        case NORTH:
            return "NORTH";
        case SOUTH:
            return "SOUTH";
        case WEST:
            return "WEST";
        case EST:
            return "EAST";
        default:
            return "UNKNOWN";
    }
}
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

/**
 * Interface for the scanning strategy
 */
class ScanningStrategy {
public:
    virtual ~ScanningStrategy() = default; // Distruttore virtuale per la classe astratta
    virtual vector<tuple<int, Path, chrono::milliseconds>> computePath(Area area, vector<DroneData> drones);
};


#endif //DRONE8_SCANNINGSTRATEGY_H
