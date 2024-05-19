#ifndef DRONE8_SCANNINGSTRATEGY_H
#define DRONE8_SCANNINGSTRATEGY_H

#include "../area/Area.hpp"
#include "../drone/Drone.h"
#include "Path.h"
#include <tuple>

using namespace std;

using DroneSchedule = tuple<int, Path, chrono::milliseconds>;

struct DroneData;

struct Coordinate {
    int x;
    int y;

    bool operator==(const Coordinate &rhs) const {
        return x == rhs.x &&
               y == rhs.y;
    }

    bool operator!=(const Coordinate &rhs) const {
        return x != rhs.x ||
               y != rhs.y;
    }
};


class Area;
/**
 * Interface for the scanning strategy
 */
class ScanningStrategy {
public:
    // virtual ~ScanningStrategy() {}; // Distruttore virtuale per la classe astratta
    virtual vector<DroneSchedule> createSchedules(Area area) = 0;

    int manhattanDistance(Coordinate a, Coordinate b) {
        return abs(a.x - b.x) + abs(a.y - b.y);
    }
};


#endif //DRONE8_SCANNINGSTRATEGY_H
