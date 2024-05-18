//
// Created by Patryk Mulica    on 01/05/24.
//

#include "Area.hpp"
#include "../drone/Drone.h"


using namespace std;



// ######## Point Class Methods #############

Point::Point() : state_(PointState::UNCHECKED) {
    start_time_ = chrono::steady_clock::now();
}

// Metodo per ottenere il tempo trascorso in millisecondi
long long Point::getElapsedTime() const {
    auto current_time = chrono::steady_clock::now();
    return chrono::duration_cast<chrono::milliseconds>(current_time - start_time_).count();
}

PointState Point::getState() const {
//     5 min in milisecondi: 5 * 60 * 1000 = 300000
    int time = static_cast<int>(300000 * TIME_ACCELERATION);
    if (state_ == PointState::UNCHECKED || getElapsedTime() > time) {
        return PointState::UNCHECKED;
    }
    return PointState::CHECKED;
}


// Metodo per aggiornare il tempo di partenza
void Point::resetTimer() {
    start_time_ = chrono::steady_clock::now();
    state_ = PointState::CHECKED;
}

// ############ AREA ############

Area::Area(int width, int height) : width_(width), height_(height) {
    // print that the area is being created
    cout << "Creating Area with width: " << width_ << " and height: " << height_ << endl;

    // create the grid with the given width and height

//    Grid grid(60, std::vector<Point>(60, Point()));
//
    grid_ = Grid(width, vector<Point>(height, Point()));

    // get height of grid_
    cout << "Area::Area: grid_.size(): " << grid_.size() << endl;

    // get width of grid_
    if (!grid_.empty()) {
        cout << "Area::Area: grid_[0].size(): " << grid_[0].size() << endl;
    }

}

int Area::getWidth() const {
    return width_;
}

int Area::getHeight() const {
    return height_;
}

void Area::setWidth(int width) {
    width_ = width;
}

void Area::setHeight(int height) {
    height_ = height;
}

const Point& Area::getPoint(int x, int y) const {
    return grid_[x][y];
}

void Area::resetPointTimer(int x, int y) {
    // check bounds
    if (x < 0 || x >= width_ || y < 0 || y >= height_) {
        return;
    }
    grid_[x][y].resetTimer();
}

string Area::toString() const {
    string result;
    for (int i = 0; i < width_; i++) {
        for (int j = 0; j < height_; j++) {
            if (grid_[i][j].getState() == PointState::CHECKED) {
                result += "X";
            } else {
                result += "O";
            }
        }
        result += "\n";
    }
    cout << "Area::toString len result: " << result.length() << endl;
    result += "\n\n\n\n\n\n";
    return result;
}

void Area::updateArea(DroneData droneData) {
//    cout << "Area::updateArea: " << droneData.x << " " << droneData.y << endl;
    int x = droneData.x;
    int y = droneData.y;

    resetPointTimer(x, y);
}

void Area::printPercentage() {
    int checked = 0;
    for (int i = 0; i < width_; i++) {
        for (int j = 0; j < height_; j++) {

            if (grid_[i][j].getState() == PointState::CHECKED) {
                checked++;
//                cout << "Checked: " << checked << endl;
            }
        }
    }
    cout << "Area::printPercentage: " << checked << " / " << width_ * height_ << " = "
         << (checked * 100) / (width_ * height_) << "%" << endl;
}
