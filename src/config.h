#ifndef DRONE8_CONFIG_H
#define DRONE8_CONFIG_H

// #define DRONE_AUTONOMY 750 // 15000/20 = 750

#define TIME_ACCELERATION 0.5    // 1 second in real life is 1 in the simulation

namespace Config {
    extern int AREA_WIDTH;
    extern int AREA_HEIGHT;

    extern int SCAN_RANGE; // in meters
    extern int DRONE_AUTONOMY; // autonomy in minutes
    extern int DRONE_SPEED; // in km/h
    extern int DRONE_STEPS; // autonomy in meters

    extern int POINT_EXPIRATION_TIME; // in milliseconds
}

#endif //DRONE8_CONFIG_H
