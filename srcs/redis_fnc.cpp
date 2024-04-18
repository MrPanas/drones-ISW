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

void deleteMessage(const std::string& stream, const std::string& messageId, redisContext *ctx) {
    // Costruire il comando XDEL
    std::string deleteCommand = "XDEL " + stream + " " + messageId;

    // Eseguire il comando XDEL
    redisReply *deleteReply = (redisReply *)redisCommand(ctx, deleteCommand.c_str());
    if (deleteReply == NULL) {
        std::cerr << "Errore nell'invio del comando XDEL" << std::endl;
    } else {
        std::cout << "Messaggio eliminato con successo" << std::endl;
        freeReplyObject(deleteReply);
    }
}

void createGroup(redisContext *ctx, const std::string& stream, const std::string& group) {
    // Costruisci il comando XGROUP CREATE
    std::string command = "XGROUP CREATE " + stream + " " + group + " $ MKSTREAM";

    // Esegui il comando XGROUP CREATE
    redisReply *reply = (redisReply *)redisCommand(ctx, command.c_str());
    if (reply == nullptr) {
        std::cerr << "Errore nell'esecuzione del comando XGROUP CREATE" << std::endl;
        return;
    }

    // Verifica se il comando è stato eseguito con successo
    if (reply->type == REDIS_REPLY_STATUS && strncmp(reply->str, "OK", 2) == 0) {
        std::cout << "Gruppo " << group << " creato con successo per lo stream " << stream << std::endl;
    } else {
        std::cerr << "Errore durante la creazione del gruppo" << std::endl;
    }

    freeReplyObject(reply);
}
