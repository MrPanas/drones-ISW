#ifndef DRONE8_SCANNINGSTRATEGY_H
#define DRONE8_SCANNINGSTRATEGY_H

#include "../area/Area.hpp"
#include "../drone/Drone.h"
#include "Path.h"

using namespace std;

using DroneSchedule = tuple<int, Path, chrono::milliseconds>;

struct DroneData;

/**
 * Interface for the scanning strategy
 */
class ScanningStrategy {
public:
    virtual ~ScanningStrategy() = default; // Distruttore virtuale per la classe astratta
    virtual vector<DroneSchedule> createSchedules(Area area);
};


#endif //DRONE8_SCANNINGSTRATEGY_H
