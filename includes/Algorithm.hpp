#include <vector>
#include <tuple>

#ifndef ALGORITHM_H
#define ALGORITHM_H

enum Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

class Path {
    public:
        Path(std::vector<std::tuple<Direction, int>> path);

        std::vector<std::tuple<Direction, int>> getPath() const;

    private:
        std::vector<std::tuple<Direction, int>> path_;
};

class Algorithm {
    public:
        virtual Path computePath(int droneId); //TODO: in input dovrebbe prendere tutte le info del drone (x, y, battery) e Field

        // Distruttore virtuale
        virtual ~Algorithm() {}
};

#endif