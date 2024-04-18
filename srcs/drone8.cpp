#include <iostream>
#include <string>
#include <hiredis/hiredis.h>
#include "Drone.hpp"
#include "ControlCenter.hpp"

int main() {
    std::cout << "Drone e ControlCenter avviati correttamente." << std::endl;
    // Creazione del drone e del control center
    Drone drone(1, 1);
    ControlCenter controlCenter(1);

    // Connessione al server Redis
    redisContext *context = redisConnect("localhost", 6379);
    if (context == NULL || context->err) {
        if (context) {
            std::cerr << "Errore di connessione: " << context->errstr << std::endl;
        } else {
            std::cerr << "Errore di allocazione del contesto della connessione" << std::endl;
        }
        return 1;
    }

    // Scrivi un messaggio sul canale cc_1
    redisReply *reply = (redisReply *)redisCommand(context, "PUBLISH cc_1 zio_can");
    if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
        std::cerr << "Errore durante l'invio del messaggio al canale" << std::endl;
        std::cerr << "Errore: " << context->errstr << std::endl;
        freeReplyObject(reply);
        redisFree(context);
        return 1;
    }
    freeReplyObject(reply);

    // Sottoscrizione al canale cc_1
    reply = (redisReply *)redisCommand(context, "SUBSCRIBE cc_1");
    if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
        std::cerr << "Errore durante la sottoscrizione al canale" << std::endl;
        freeReplyObject(reply);
        redisFree(context);
        return 1;
    }
    freeReplyObject(reply);

    // Leggi e stampa tutti i messaggi già presenti nel canale
    while (true) {
        reply = (redisReply *)redisCommand(context, "GET cc_1");
        if (reply != NULL && reply->type == REDIS_REPLY_STRING) {
            std::cout << "Messaggio sul canale cc_1: " << reply->str << std::endl;
            freeReplyObject(reply);
        } else {
            break;
        }
    }

    std::cout << "In ascolto sul canale cc_1. Premi Ctrl+C per interrompere." << std::endl;

    // Loop di ascolto dei messaggi
    while (true) {
        redisReply *reply = NULL;
        if (redisGetReply(context, (void **)&reply) == REDIS_OK) {
            if (reply != NULL && reply->type == REDIS_REPLY_ARRAY && reply->elements == 3) {
                std::cout << "Messaggio ricevuto sul canale " << reply->element[1]->str << ": " << reply->element[2]->str << std::endl;
            }
            freeReplyObject(reply);
        }
    }

    // Chiusura della connessione
    redisFree(context);

    return 0;
}