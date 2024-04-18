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
    std::cout << "Comando inviato: " << reply_ << std::endl;
    return reply_ != NULL;
}

void RedisClient::getReply() {
    print_reply(reply_);
    freeReplyObject(reply_);
    reply_ = nullptr;
}

void RedisClient::print_reply(redisReply *reply) {
    if (reply == NULL) {
        printf("Risposta nullo\n");
        return;
    }

    if (reply->type == REDIS_REPLY_ARRAY) {
        printf("L'array ha %zu elementi:\n", reply->elements);
        for (size_t i = 0; i < reply->elements; ++i) {
            // printf("Elemento %zu:\n", i);
            print_reply(reply->element[i]);
        }
    } else if (reply->type == REDIS_REPLY_STRING) {
        printf("Stringa: %s\n", reply->str);
    } else if (reply->type == REDIS_REPLY_INTEGER) {
        printf("Inter: %lld\n", reply->integer);
    } else if (reply->type == REDIS_REPLY_ERROR) {
        printf("Errore: %s\n", reply->str);
    } else {
        printf("Tipo di risposta non gestito\n");
    }
}









bool RedisClient::hasReply() {
    return reply_ != nullptr;
}