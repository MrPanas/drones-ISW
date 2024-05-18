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
#include "../scanning_strategy/config.h"

using namespace std;

using IntGrid = vector<vector<int>>;

struct DroneData;


enum PointState {
    CHECKED,
    UNCHECKED,


};

// create PointState comparison operator



/**
 * Class that represents a point in the grid
 */
class Point {
public:
    Point();
    long long getElapsedTime() const; // const perchè non modifica lo stato dell'oggetto
    void resetTimer();
    PointState getState() const;


private:
    chrono::steady_clock::time_point start_time_;
    PointState state_;
};

using Grid = vector<vector<Point>>;
/**
 * Class that represents the area
 */
class Area {
public:
    Area(int width, int height);
    int getWidth() const;
    int getHeight() const;
    void setWidth(int width);
    void setHeight(int height);
    pair<int, int> getCCPosition() const; // TODO:implement

    string toString() const;

    const Point& getPoint(int x, int y) const;
    void resetPointTimer(int x, int y);
    void updateArea(DroneData data);
    void printPercentage();

private:
    int width_;
    int height_;
    Grid grid_;



};



#endif //DRONE8_AREA_HPP
