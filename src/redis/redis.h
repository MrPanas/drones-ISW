//
// Created by Patryk Mulica    on 02/05/24.
//

#ifndef DRONE8_REDIS_H
#define DRONE8_REDIS_H

#include <hiredis.h>
#include <iostream>
#include <vector>
#include <nlohmann/json.hpp>


using namespace std;
using json = nlohmann::json;
using  Response =  tuple<string,map<string,string>>;
using Message = map<string,string>;

struct GroupInfo {
    string name;
    long consumers{};
    long pending{};
    string lastDeliveredId;
    long entriesRead{};
    long lag{};
};

string createGroup(redisContext *context, const string &stream, const string &group, bool mkstream);

long destroyGroup(redisContext *context, const string &stream, const string &group);

long deleteStream(redisContext *context, const string &stream);

string sendMessage(redisContext *context, const string &stream, Message &message);

string sendJsonMessage(redisContext *context, const string &stream, const json &jsonObject);

long deleteStream(redisContext *context, const string &stream);


Response readMessageGroup(redisContext *context, const string &group, const string &consumer, const string &stream, int block);

long ackMessage(redisContext *context, const string &stream, const string &group, const string &messageId);

GroupInfo getInfoGroup(redisContext *context, const string &stream, const string &groupName);

#endif //DRONE8_REDIS_H