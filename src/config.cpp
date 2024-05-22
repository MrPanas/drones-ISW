#include "config.h"

namespace Config {
    int AREA_WIDTH = 200;
    int AREA_HEIGHT = 200;

    int SCAN_RANGE = 10;
    int DRONE_AUTONOMY = 10;
    int DRONE_SPEED = 30;
    int DRONE_STEPS = 15000;
    int MINIMUM_CHARGING_TIME = 7200000;
    int MAXIMUM_CHARGING_TIME = 10800000;

    int POINT_EXPIRATION_TIME = 300000; // 300000 milliseconds = 5 minutes
    int NUMBER_OF_DRONES = 9000;

    unsigned int TIME_TO_SCAN = 220; // in minutes
}