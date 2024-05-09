//
// Created by Patryk Mulica    on 01/05/24.
//

#include "Area.hpp"

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
    if (state_ == PointState::UNCHECKED || getElapsedTime() > 5000) {
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
    grid_.resize(width_);
    // width and height in meters
    for (int i = 0; i < width_; i++) {
        grid_[i].resize(height_);
        for (int j = 0; j < height_; j++) {
            // Init the grid
            grid_[i][j] = Point();
        }
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



