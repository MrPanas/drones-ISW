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
    virtual ~ScanningStrategy() = default; // Distruttore virtuale per la classe astratta
    virtual vector<DroneSchedule> createSchedules(Area area);
    tuple<Coordinate, int> getAvailableCoord(int autonomy, Coordinate current_position, Coordinate next_position, Coordinate cc_pos);
    tuple<Coordinate, int> goSouth(int autonomy, Coordinate current_position, Coordinate end, Coordinate cc_pos);
    Path returnToCC(int autonomy, Coordinate current_position, Coordinate cc_pos);

    int manhattanDistance(Coordinate a, Coordinate b);
};


#endif //DRONE8_SCANNINGSTRATEGY_H
