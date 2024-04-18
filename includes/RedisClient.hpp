#ifndef REDIS_CLIENT_HPP
#define REDIS_CLIENT_HPP

#include <hiredis/hiredis.h>
#include <string>
#include <vector>

class RedisClient {
public:
    RedisClient(const std::string& hostname = "localhost", int port = 6379);
    ~RedisClient();

    bool sendCommand(const std::string& command);
    void getReply();
    void print_reply(redisReply *reply);
    bool hasReply();

private:
    redisContext* context_;
    redisReply* reply_;
};

#endif