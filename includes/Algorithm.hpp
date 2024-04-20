#include <vector>
#include <tuple>
#include "Field.hpp"
#include "Drone.hpp"

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
        //                int: drone_id
        virtual std::vector<int, Path> computePath(Field field, std::vector<<DroneData> drones); 

        // TODO: ci dovrà essere una funzione che decide ogni quanto tempo chiamare computePath

        // Distruttore virtuale
        virtual ~Algorithm() {}
};

#endif