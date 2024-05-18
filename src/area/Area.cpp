//
// Created by Patryk Mulica    on 01/05/24.
//

#include "Area.hpp"
#include "../drone/Drone.h"


using namespace std;


// ############ AREA ############



Area::Area(int width, int height) : width_(width), height_(height) {
    // print that the area is being created
    cout << "Creating Area with width: " << width_ << " and height: " << height_ << endl;

    // create the grid with the given width and height

    // timestamp now - expiration time
    auto expired = chrono::system_clock::now() - chrono::milliseconds(static_cast<long>(ceil(Config::POINT_EXPIRATION_TIME * TIME_ACCELERATION)));
    grid_ = Grid(width, vector<Timestamp>(height, expired));

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

Grid Area::getGrid() const {
    return grid_;
}

string Area::toString() const {
    string result;
    // timestamp now
    auto now = chrono::system_clock::now();
    for (int i = 0; i < width_; i++) {
        for (int j = 0; j < height_; j++) {
            if (now - grid_[i][j] > chrono::milliseconds(static_cast<long>(ceil(Config::POINT_EXPIRATION_TIME * TIME_ACCELERATION)))) {
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

void Area::updatePoint(int x, int y) {
    if (x < 0 || x >= width_ || y < 0 || y >= height_) {
        return;
    }
    // timestamp now
    auto now = chrono::system_clock::now();
    grid_[x][y] = now;
}

void Area::printPercentage() {
    int checked = 0;
    auto now = chrono::system_clock::now();
    for (int i = 0; i < width_; i++) {
        for (int j = 0; j < height_; j++) {

            if (now - grid_[i][j] < chrono::milliseconds(static_cast<long>(ceil(Config::POINT_EXPIRATION_TIME * TIME_ACCELERATION)))) {
                checked++;
            }
        }
    }
    cout << "Area::printPercentage: " << checked << " / " << width_ * height_ << " = "
         << (checked * 100) / (width_ * height_) << "%" << endl;
}
