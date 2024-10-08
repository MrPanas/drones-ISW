//
// Created by Patryk Mulica    on 01/05/24.
//

#include "Area.hpp"



using namespace std;


// ############ AREA ############


Area::Area(int id, int width, int height) : id_(id), width_(width), height_(height) {
    // timestamp now - expiration time
    long expired =
            chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count()
            -
            static_cast<long>(ceil(Config::POINT_EXPIRATION_TIME * TIME_ACCELERATION));

    grid_ = Grid(width, vector<long>(height, expired));

    // get height of grid_
    cout << "Area::Area: grid_.size(): " << grid_.size() << endl;

    // get width of grid_
    if (!grid_.empty()) {
        cout << "Area::Area: grid_[0].size(): " << grid_[0].size() << endl;
    }

}

int Area::getId() const {
    return id_;
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
    auto nowT = chrono::system_clock::now();
    long now = chrono::duration_cast<chrono::milliseconds>(nowT.time_since_epoch()).count();
    for (int i = 0; i < width_; i++) {
        for (int j = 0; j < height_; j++) {
            if (now - grid_[i][j] > static_cast<long>(ceil(Config::POINT_EXPIRATION_TIME * TIME_ACCELERATION))) {
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

void Area::updatePoint(int x, int y, long timestamp) {
    if (x < 0 || x >= width_ || y < 0 || y >= height_) {
        return;
    }
    // timestamp now
//    auto now = chrono::system_clock::now();
    grid_[y][x] = (grid_[y][x] > timestamp) ? grid_[y][x] : timestamp;
}

float Area::getPercentage() {
    int checked = 0;
    auto nowT = chrono::system_clock::now();
    // convert to long
    long now = chrono::duration_cast<chrono::milliseconds>(nowT.time_since_epoch()).count();
    for (int i = 0; i < width_; i++) {
        for (int j = 0; j < height_; j++) {

            if (now - grid_[i][j] < static_cast<long>(ceil(Config::POINT_EXPIRATION_TIME * TIME_ACCELERATION))) {
                checked++;
            }
        }
    }

    return static_cast<float>(checked) / static_cast<float>((width_ * height_));
}
