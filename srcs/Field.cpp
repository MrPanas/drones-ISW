#include "Field.hpp"

Field::Field(int width, int height) : width_(width), height_(height) {
    // width and height in meters
    
    // Inizializza la matrice di punti
    for (int i = 0; i < width_; i++) {
        for (int j = 0; j < height_; j++) {
            points_[i][j] = Point(i, j);
        }
    }
}

int Field::getWidth() const {
    return width_;
}

int Field::getHeight() const {
    return height_;
}

void Field::setWidth(int width) {
    width_ = width;
}

void Field::setHeight(int height) {
    height_ = height;
}

const Point& Field::getPoint(int x, int y) const {
    return points_[x][y];
}

std::pair<int, int> Field::getCCPosition() const {
    return std::make_pair(width_ / 2, height_ / 2);
}

void Field::resetPointTimer(int x, int y) {
    points_[x][y].resetTimer();
}

Point::Point(int x, int y) : x_(x), y_(y), state_(PointState::UNCHECKED) {
    start_time_ = std::chrono::steady_clock::now();
}

int Point::getX() const {
    return x_;
}

int Point::getY() const {
    return y_;
}

 // Metodo per ottenere il tempo trascorso in millisecondi
long long Point::getElapsedTime() const {
    auto current_time = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time_).count();
}

PointState Point::getState() const {
    if (state_ == PointState::UNCHECKED || getElapsedTime() > 5000) {
        return PointState::UNCHECKED;
    }
    return PointState::CHECKED;
}


// Metodo per aggiornare il tempo di partenza
void Point::resetTimer() {
    start_time_ = std::chrono::steady_clock::now();
    state_ = PointState::CHECKED;
}
