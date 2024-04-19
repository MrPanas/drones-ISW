#ifndef CONTROLCENTER_HPP
#define CONTROLCENTER_HPP

#include "Algorithm.hpp"
#include <hiredis/hiredis.h>
#include "RedisClient.hpp"
#include <chrono>

class ControlCenter {
    public:
        ControlCenter(int id);

        void setAlgorithm(Algorithm* alg);

        void handleDroneRequests();
        void sendPath(int droneId, std::string& path);
        void powerOn();
        void powerOff();

    private:
        const int id_;
        Algorithm* algorithm;
        RedisClient redisClient_;
        std::string channelName;
        redisContext *ctx_;
};

class Field { //TODO: Creare un altro file con Field e Point ??
    public:
        Field(int width, int height);
        int getWidth() const;
        int getHeight() const;
        void setWidth(int width);
        void setHeight(int height);

        const Point& getPoint(int x, int y) const;
        void resetPointTimer(int x, int y);
};

class Point {
    public:
        Point(int x, int y);
        int getX() const;
        int getY() const;
        long long getElapsedTime() const; // const perchè non modifica lo stato dell'oggetto
        void resetTimer();


    private:
        int x_;
        int y_;
        std::chrono::steady_clock::time_point start_time_;
};

#endif

