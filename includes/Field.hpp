#ifndef FIELD_HPP
#define FIELD_HPP

#include <chrono>
#include <vector>

enum PointState {
    CHECKED,
    UNCHECKED,
};

class Point {
    public:
        Point(int x, int y);
        int getX() const;
        int getY() const;
        long long getElapsedTime() const; // const perchè non modifica lo stato dell'oggetto
        void resetTimer();
        PointState getState() const;
        

    private:
        int x_;
        int y_;
        std::chrono::steady_clock::time_point start_time_;
        PointState state_;
};

class Field { 
    public:
        Field(int width, int height);
        int getWidth() const;
        int getHeight() const;
        void setWidth(int width);
        void setHeight(int height);

        const Point& getPoint(int x, int y) const;
        void resetPointTimer(int x, int y);

    private:
        int width_;
        int height_;
        std::vector<std::vector<Point>> points_;
};



#endif