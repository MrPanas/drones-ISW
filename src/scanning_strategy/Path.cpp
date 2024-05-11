#include "Path.h"

#include <utility>


Path::Path(vector<tuple<Direction, int>> path) : path_(path) {}

Path::Path() : path_(std::vector<std::tuple<Direction, int>>()) {}

vector<tuple<Direction, int>> Path::getPath() const {
    return path_;
}

void Path::addDirection(Direction dir, int steps) {
    path_.emplace_back(dir, steps);
}

void Path::addPath(const Path& path) {
    for (const auto& [direction, steps] : path.getPath()) {
        path_.emplace_back(direction, steps);
    }
}

string Path::toString() const {
    string result;
    for (const auto& [direction, steps] : path_) {
        result += ::toString(direction) + to_string(steps) + "_";
    }
    return result;
}
