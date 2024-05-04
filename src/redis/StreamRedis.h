//
// Created by Patryk Mulica    on 01/05/24.
//

#ifndef DRONE8_STREAMREDIS_H
#define DRONE8_STREAMREDIS_H


#include <hiredis.h>
#include <iostream>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#define REDIS_SERVER "localhost"
#define REDIS_PORT 6379



using namespace std;

class StreamRedis {
private:


public:
    redisContext *context_;
    // Constructor
    StreamRedis();

    // Print a stream from redis
    void printStream(const string &stream);

    // Send a message to a stream
    void sendMessage(const string &stream, const vector<string> &message);

    // Destructor
    ~StreamRedis() {
        // Free the redis context
        redisFree(context_);
        // Successfully disconnected from redis server
        cout << "StreamRedis::~StreamRedis: Successfully disconnected from redis server" << endl;
    }
    // send Json message to a stream
    void sendJsonMessage(const string &stream, const json &jsonObject);

    // retrieve a message from a stream and convert it to json
    json convertMessageToJson(redisReply *message);

    void uploadLastMessage(const string &stream, json &jsonObject);


    void createGroup(const string &stream, const string &group);

    int consumeGroup(const string &stream, const string &group, const string &consumer, json &jsonObject);
};




#endif //DRONE8_STREAMREDIS_H
