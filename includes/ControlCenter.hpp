#ifndef CONTROLCENTER_HPP
#define CONTROLCENTER_HPP

#include "Algorithm.hpp"
#include <hiredis/hiredis.h>
#include "RedisClient.hpp"

class ControlCenter {
    public:
        ControlCenter(int id);

        void setAlgorithm(Algorithm* alg);

    private:
        const int id_;
        Algorithm* algorithm;
        RedisClient redisClient_;
        std::string channelName;
};

#endif

