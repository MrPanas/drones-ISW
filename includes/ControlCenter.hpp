#ifndef CONTROLCENTER_HPP
#define CONTROLCENTER_HPP

#include "Algorithm.hpp"
#include <hiredis/hiredis.h>
#include "RedisClient.hpp"

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
        void setTimerToMax(int x, int y); // TODO
};

class Point {
    public:
        Point(int x, int y);
        int getX() const;
        int getY() const;
        int timer; // TODO non sarà int ma un tipo di dato che rappresenta il tempo

    private:
        int x_;
        int y_;
};

#endif

