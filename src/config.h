#ifndef DRONE8_CONFIG_H
#define DRONE8_CONFIG_H

// #define DRONE_AUTONOMY 750 // 15000/20 = 750

#define TIME_ACCELERATION 1   // 1 second in real life is 1 in the simulation

namespace Config {
    extern int AREA_WIDTH; // in meters
    extern int AREA_HEIGHT; // in meters

    extern int SCAN_RANGE; // in meters
    extern int DRONE_AUTONOMY; // autonomy in minutes
    extern int DRONE_SPEED; // in km/h
    extern int DRONE_STEPS; // autonomy in meters
    extern int MINIMUM_CHARGING_TIME; // in milliseconds
    extern int MAXIMUM_CHARGING_TIME; // in milliseconds

    extern int POINT_EXPIRATION_TIME; // in milliseconds
    extern int NUMBER_OF_DRONES;

    extern unsigned int TIME_TO_SCAN; // in minutes
}

#endif //DRONE8_CONFIG_H
