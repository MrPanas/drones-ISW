//
// Created by Patryk Mulica    on 01/05/24.
//

#ifndef DRONE8_AREA_HPP
#define DRONE8_AREA_HPP

#include <vector>
#include <iostream>
#include <chrono>
#include "../drone/Drone.h"
#include <mutex>
#include "../config.h"

using namespace std;

struct DroneData;


using Timestamp = chrono::system_clock::time_point;

using Grid = vector<vector<long>>;
/**
 * Class that represents the area
 */
class Area {
public:
    Area(int id, int width, int height);
    int getId() const;
    int getWidth() const;
    int getHeight() const;
    Grid getGrid() const;
    void setWidth(int width);
    void setHeight(int height);

    string toString() const;

    void updatePoint(int x, int y, long timestamp);
    float getPercentage();

private:
    int width_;
    int height_;
    Grid grid_;
    int id_;



};



#endif //DRONE8_AREA_HPP
