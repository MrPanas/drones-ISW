#include "redis_fnc.hpp"
#include <iostream>

json redisReplyToJSON(redisReply *reply) {
    json jsonObject;

    // stampa dati di reply
    if (reply == NULL) {
        std::cout << "Risposta nullo" << std::endl;
        return jsonObject;
    }

    if (reply != nullptr && reply->type == REDIS_REPLY_ARRAY) {
        std::cout << "L'array ha " << reply->elements << " elementi" << std::endl;
        // Estrai i dati dall'array di risposta
        std::string id = reply->element[0]->str;
        redisReply *data = reply->element[1];
        if (data != nullptr && data->type == REDIS_REPLY_ARRAY && data->elements % 2 == 0) {
            // Costruisci l'oggetto JSON
            jsonObject["id"] = id;
            for (size_t i = 0; i < data->elements; i += 2) {
                std::string key = data->element[i]->str;
                redisReply *value = data->element[i + 1];
                if (value != nullptr) {
                    if (value->type == REDIS_REPLY_STRING) {
                        jsonObject["data"][key] = value->str;
                    } else if (value->type == REDIS_REPLY_INTEGER) {
                        jsonObject["data"][key] = value->integer;
                    }
                }
            }
        }
    }
    return jsonObject;
}