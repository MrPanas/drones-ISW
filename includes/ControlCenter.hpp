#include "Algorithm.hpp"
#include "MapToScan.hpp"
#include <hiredis/hiredis.h>

#ifndef CONTROLCENTER_HPP
#define CONTROLCENTER_HPP

class ControlCenter {
    public:
        ControlCenter(int id);

        void setAlgorithm(Algorithm* alg);
        void setMap(MapToScan map);

    private:
        const int id_;
        MapToScan map;
        Algorithm* algorithm;
};

#endif

