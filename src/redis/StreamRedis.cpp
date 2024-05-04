
#include "StreamRedis.h"

// Query to get the last two messages from a stream (from the end to the beginning)
// XREVRANGE %s + - COUNT 2
#include <vector>



// Structure of redis reply
// Message id
// 1714574161939-0
//        json
//        {"battery":100,"id":1,"status":"OK"}

// Constructor
StreamRedis::StreamRedis() {
    context_ = redisConnect(REDIS_SERVER, REDIS_PORT);
    if (context_ == NULL || context_->err) {
        if (context_) {
            std::cerr << "StreamRedis::StreamRedis: Error: " << context_->errstr << std::endl;
            redisFree(context_);
        } else {
            std::cerr << "StreamRedis::StreamRedis: Error: Can't allocate redis context" << std::endl;
        }
    }
    // Successfully connected to redis server
    std::cout << "StreamRedis::StreamRedis: Successfully connected to redis server" << std::endl;

}
void printRedisReplyDetails(redisReply* reply, int level = 0) {
    if (reply == nullptr) {
        std::cerr << "La risposta è nulla" << std::endl;
        return;
    }

    // Stampa il livello di annidamento
//    for (int i = 0; i < level; ++i) {
//        std::cout << " ";
//    }

//    std::cout << "Tipo di risposta: ";
    switch (reply->type) {
        case REDIS_REPLY_STRING:
//            std::cout << "Stringa" << std::endl;
            // Stampa il contenuto della stringa con l'indentazione appropriata
            for (int i = 0; i < level + 1; ++i) {
                std::cout << "  ";
            }
            std::cout << reply->str << std::endl;
            break;

        case REDIS_REPLY_ARRAY:

//            std::cout << "Array" << std::endl;
//            for (int i = 0; i < level + 1; ++i) {
//                std::cout << "  ";
//            }
//            std::cout << "Numero di elementi: " << reply->elements << std::endl;
            // Se desideri stampare dettagli specifici per ogni elemento nell'array, puoi aggiungere qui la logica
            for (size_t i = 0; i < reply->elements; ++i) {
                printRedisReplyDetails(reply->element[i], level+1); // Chiamata ricorsiva per stampare dettagli specifici per ciascun elemento
            }
            break;

        case REDIS_REPLY_INTEGER:
            std::cout << "Interger" << std::endl;
            // Stampa il valore intero con l'indentazione appropriata
            for (int i = 0; i < level + 1; ++i) {
                std::cout << "  ";
            }
            std::cout << "Valore: " << reply->integer << std::endl;
            break;

        case REDIS_REPLY_NIL:
            std::cout << "Nullo" << std::endl;
            break;

        case REDIS_REPLY_STATUS:
            std::cout << "Status" << std::endl;
            // Stampa il contenuto dello status con l'indentazione appropriata
            for (int i = 0; i < level + 1; ++i) {
                std::cout << "  ";
            }
            std::cout << "Contenuto: " << reply->str << std::endl;
            break;

        case REDIS_REPLY_ERROR:
            std::cout << "Errore" << std::endl;
            // Stampa il messaggio di errore con l'indentazione appropriata
            for (int i = 0; i < level + 1; ++i) {
                std::cout << "  ";
            }
            std::cout << "Messaggio di errore: " << reply->str << std::endl;
            break;

        default:
            std::cout << "Tipo sconosciuto" << std::endl;
            break;
    }
}


void StreamRedis::printStream(const std::string &stream) {
    // Try to get the stream
    auto *reply = (redisReply *) redisCommand(context_, "XREVRANGE %s + - COUNT 2", stream.c_str());

    if (reply == NULL) {
        std::cerr << "StreamRedis::printStream: Error: " << context_->errstr << std::endl;
        return;
    }
    // print reply type
    std::cout << "StreamRedis::printStream: Reply type: " << reply->type << std::endl;
    // print reply details
        printRedisReplyDetails(reply);





    freeReplyObject(reply);

}

// Create a function that sends a message to a stream
// takes a list of strings as arguments
// returns a string that is the query to send to redis
// DEPRACTED
std::string createMessageQuery(const std::string &stream, const std::vector<std::string> &message) {
    std::string query = "XADD " + stream + " * ";
    for (int i = 0; i < message.size(); i += 2) {
        query += message[i] + " " + message[i + 1] + " ";
    }
    return query;
}

// DEPRECATED
void StreamRedis::sendMessage(const std::string &stream, const std::vector<std::string> &message) {
    // Send the message to the stream
    // Create the query
    std::string query = createMessageQuery(stream, message);
    std::cout << "StreamRedis::sendMessage: Sending a message to the stream" << std::endl;
    auto *reply = (redisReply *) redisCommand(context_, query.c_str());
    if (reply == NULL) {
        std::cerr << "StreamRedis::sendMessage: Error: " << context_->errstr << std::endl;
        return;
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        std::cerr << "StreamRedis::sendMessage: Error: " << reply->str << std::endl;
    }
    freeReplyObject(reply);

}


void StreamRedis::sendJsonMessage(const std::string &stream, const json &jsonObject) {
    //print the json object
    std::cout << "StreamRedis::sendJsonMessage: Sending a json message to the stream" << std::endl;
    std::cout << jsonObject.dump(2) << std::endl;

    // Send the json object to the stream
    std::string query = "XADD " + stream + " * json " + jsonObject.dump();
    auto *reply = (redisReply *) redisCommand(context_, query.c_str());
    if (reply == NULL) {
        std::cerr << "StreamRedis::sendJsonMessage: Error: " << context_->errstr << std::endl;
        return;
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        std::cerr << "StreamRedis::sendJsonMessage: Error: " << reply->str << std::endl;
    }

    freeReplyObject(reply);
}

// message is an atomic redis reply of this type:
// 1714574161939-0 (redis message id)
//        json
//        {"battery":100,"id":1,"status":"OK"}
json StreamRedis::convertMessageToJson(redisReply *message) {
    std::string messageId = message->element[0]->str;
    std::string messageJson = message->element[1]->element[1]->str;

    return json::parse(messageJson);
}

void StreamRedis::uploadLastMessage(const std::string &stream, json &jsonObject) {
    // Get the last message from the stream
    auto *reply = (redisReply *) redisCommand(context_, "XREVRANGE %s + - COUNT 1", stream.c_str());
    if (reply == NULL) {
        std::cerr << "StreamRedis::selectLastMessage: Error: " << context_->errstr << std::endl;
        return;
    }
    if (reply->type != REDIS_REPLY_ARRAY || reply->elements == 0) {
        std::cerr << "StreamRedis::selectLastMessage: Error: Expected an array or empty" << std::endl;
        freeReplyObject(reply);
        return;
    }
    // get element
    auto *message = reply->element[0];
    jsonObject = convertMessageToJson(message);

    freeReplyObject(reply);

}








