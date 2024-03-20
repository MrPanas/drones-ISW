#include <vector>
#include <tuple>
#include "Direction.hpp"

#ifndef ALGORITHM_H
#define ALGORITHM_H

class Algorithm {
    public:
        virtual std::vector<std::tuple<Direction, int>> computePath(int droneId);

        // Distruttore virtuale
        virtual ~Algorithm() {}
};

#endif