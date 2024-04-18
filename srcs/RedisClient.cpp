#include "RedisClient.hpp"
#include <stdexcept>
#include <iostream>


RedisClient::RedisClient(const std::string& hostname, int port) {
    context_ = redisConnect(hostname.c_str(), port);
    if (context_ == NULL || context_->err) {
        throw std::runtime_error("Errore di connessione a Redis");
    }
}

RedisClient::~RedisClient() {
    if (context_ != NULL) {
        std::cout << "Chiusura della connessione a Redis!!" << std::endl;
        // redisFree(context_);
    }
}

bool RedisClient::sendCommand(const std::string& command) {
    reply_ = (redisReply*)redisCommand(context_, command.c_str());
    return reply_ != NULL;
}

std::string RedisClient::getReply() {
    if (reply_ == NULL) {
        throw std::runtime_error("Nessuna risposta disponibile");
    }
    std::string result(reply_->str);
    freeReplyObject(reply_);
    return result;
}