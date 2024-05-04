
#include <iostream>
#include "Drone.h"

Drone::Drone(unsigned int id) : id_(id){
    ctx_ = redisConnect(REDIS_HOST, stoi(REDIS_PORT));
    if (ctx_ == NULL || ctx_->err) {
        if (ctx_) {
            std::cout << "Error: " << ctx_->errstr << std::endl;
            redisFree(ctx_);
        } else {
            std::cout << "Can't allocate redis context" << std::endl;
        }
    }

    createGroup(ctx_, "drone_" + to_string(id_), "Drone_" + to_string(id_) , true);
}

Drone::Drone(unsigned int id, unsigned int cc_id) : Drone(id) {
    cc_id_ = cc_id;
}
