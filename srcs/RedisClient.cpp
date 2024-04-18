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

std::string RedisClient::getReply() {
    std::cout << "Ricezione della risposta..." << reply_ << std::endl;
    // Verifica se il reply_ è valido
    if (reply_ == NULL || reply_->type != REDIS_REPLY_ARRAY) {
        throw std::runtime_error("Errore nella ricezione della risposta XREAD");
    }

    std::string result;

    // Itera attraverso gli elementi dell'array di risposta
    for (size_t i = 0; i < reply_->elements; i++) {
        redisReply* messageArray = reply_->element[i];
        // Verifica se l'elemento è un array valido
        if (messageArray == NULL || messageArray->type != REDIS_REPLY_ARRAY) {
            continue; // Ignora gli elementi non validi
        }

        // Itera attraverso gli elementi dell'array di messaggi
        for (size_t j = 0; j < messageArray->elements; j++) {
            redisReply* messageElement = messageArray->element[j];
            // Verifica se l'elemento è una stringa
            if (messageElement != NULL && messageElement->type == REDIS_REPLY_STRING) {
                result += messageElement->str;
                result += " "; // Aggiungi uno spazio tra i campi del messaggio
            }
        }
    }

    // Libera la memoria allocata per la risposta
    freeReplyObject(reply_);
    reply_ = nullptr;

    return result;
}



bool RedisClient::hasReply() {
    return reply_ != nullptr;
}