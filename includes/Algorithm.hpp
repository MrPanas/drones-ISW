#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <vector>
#include <tuple>
#include "Drone.hpp"
#include "Field.hpp"

enum Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

class Path {
    public:
        Path(std::vector<std::tuple<Direction, int>> path);
        Path();

        std::vector<std::tuple<Direction, int>> getPath() const;

        void addDirection(Direction dir, int steps);

    private:
        std::vector<std::tuple<Direction, int>> path_;
};
struct DroneData;

class Algorithm {
    public:
        DroneData droneData();
        //                          int: drone_id
        virtual std::vector<std::tuple<int, Path>> computePath(Field field, std::vector<DroneData> drones); 

        // TODO: ci dovrà essere una funzione che decide ogni quanto tempo chiamare computePath

        // Distruttore virtuale
        virtual ~Algorithm() {}
};

#endif