#include <vector>
#include <tuple>

#ifndef ALGORITHM_H
#define ALGORITHM_H

class Algorithm {
    public:
        virtual Path computePath(int droneId); //TODO: in input dovrebbe prendere tutte le info del drone oppure tutte le info della mappa?

        // Distruttore virtuale
        virtual ~Algorithm() {}
};

class Path {
    public:
        Path(std::vector<std::tuple<Direction, int>> path);

        std::vector<std::tuple<Direction, int>> getPath() const;

    private:
        std::vector<std::tuple<Direction, int>> path_;
};

enum Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

#endif