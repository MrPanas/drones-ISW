#ifndef CONTROLCENTER_HPP
#define CONTROLCENTER_HPP

#include "Algorithm.hpp"
#include "MapToScan.hpp"
#include <hiredis/hiredis.h>

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

