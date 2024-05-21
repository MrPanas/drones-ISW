//
// Created by Patryk Mulica    on 02/05/24.
//
#include "redis.h"


//XACK                      Done
//XADD                      Done
//XAUTOCLAIM                ?
//XCLAIM                    ?
//XDEL                      ?
//XGROUP CREATE             OK
//XGROUP CREATECONSUMER     Inutile
//XGROUP DELCONSUMER        ?
//XGROUP DESTROY            Done
//XGROUP SETID              ?
//XINFO CONSUMERS           ?
//XINFO GROUPS              Done
//XINFO STREAM              ?
//XLEN                      ?
//XPENDING                  ?
//XRANGE                    Da fare?
//XREAD                     Da fare?
//XREADGROUP                Done
//XREVRANGE                 Da fare?
//XSETID                    ?
//XTRIM                     ?


/**
 * Create a group
 * @param context redis context
 * @param stream stream name
 * @param group group name
 * @param mkstream create the stream if it does not exist
 * @return Empty string if noReply OK if the group was created successfully or an error message
 */
string Redis::createGroup(redisContext *context, const string &stream, const string &group, bool mkstream = true) {
    string mkstreamStr = mkstream ? "MKSTREAM" : "";
    // Create a group
    auto *reply = (redisReply *) redisCommand(context, "XGROUP CREATE %s %s $ MKSTREAM",
                                              stream.c_str(), group.c_str(), mkstreamStr.c_str());
    if (reply == nullptr) {
        cerr << "createGroup: Error: " << context->errstr << endl;
        return "";
    }
    string result = reply->str;
    freeReplyObject(reply);
    return result;
}


/**
 * Delete a group
 * @param context redis context
 * @param stream stream name
 * @param group group name
 * @return true if the group was deleted successfully
 */
long Redis::destroyGroup(redisContext *context, const string &stream, const string &group) {
    // Create a group
    auto *reply = (redisReply *) redisCommand(context, "XGROUP DESTROY %s %s",
                                              stream.c_str(), group.c_str());
    if (reply == nullptr) {
        cerr << "deleteGroup: Error: " << context->errstr << endl;
        return -1;
    }

    if (reply->type == REDIS_REPLY_ERROR) {
        cerr << "deleteGroup: Error: " << reply->str << endl;

        //        return "";
    }
    long result = reply->integer;
    freeReplyObject(reply);
    return result;
}

/**
 * Send a message to a stream
 * @param context redis context
 * @param stream stream name
 * @param message message to send
 * @return message id
 */
string Redis::sendMessage(redisContext *context, const string &stream, Message &message) {
    // Create XADD command
    string xadd = "XADD " + stream + " * ";

    // add the message to the command
    for (const auto &item: message) {
        xadd += item.first + " " + item.second + " ";
    }

    // Send the message
    auto *reply = (redisReply *) redisCommand(context, xadd.c_str());

    if (reply == nullptr) {
        cerr << "sendMessage: Error: " << context->errstr << endl;
        cout << "sendMessageXADD: " << xadd << endl;
        return "";
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        cerr << "sendMessage: Error: " << reply->str << endl;
        cout << "sendMessageXADD: " << xadd << endl;
        freeReplyObject(reply);
        return "";
    }

    string messageId = reply->str;
    freeReplyObject(reply);
    return messageId;
}





// XDEL key id [id ...]

/**
 * Delete a message from a stream
 * @param context redis context
 * @param stream stream name
 * @param messageId message id
 * @return number of messages deleted
 */
long Redis::deleteMessage(redisContext *context, const string &stream, const string &messageId) {
    // Delete a message
    // convert to char*
    char *streamChr = new char[stream.length() + 1];
    strcpy(streamChr, stream.c_str());
    char *messageIdChr = new char[messageId.length() + 1];
    strcpy(messageIdChr, messageId.c_str());

    auto *reply = (redisReply *) redisCommand(context, "XDEL %s %s", streamChr, messageIdChr);
    delete[] streamChr;
    delete[] messageIdChr;

    if (reply == nullptr) {
        cerr << "deleteMessage: Error: " << context->errstr << endl;
        return -1;
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        cerr << "deleteMessage: Error: " << reply->str << endl;
        freeReplyObject(reply);
        return -1;
    }
    long result = reply->integer;
    freeReplyObject(reply);

    return result;
}


/**
 * Send a json message to a stream
 * @param context redis context
 * @param stream stream name
 * @param jsonObject json object
 * @return message id
 */
string Redis::sendJsonMessage(redisContext *context, const string &stream, const json &jsonObject) {
    // Send the json object to the stream
    auto *reply = (redisReply *) redisCommand(context, "XADD %s * json %s", stream.c_str(),
                                              jsonObject.dump().c_str());
    if (reply == nullptr) {
        cerr << "sendJsonMessage: Error: " << context->errstr << endl;
        return "";
    }
    string messageId = reply->str;
    freeReplyObject(reply);
    return messageId;
}

/**
 * Delete a stream
 * @param context redis context
 * @param stream stream name
 * @return number of messages deleted
 */
long Redis::deleteStream(redisContext *context, const string &stream) {
    // Create a group
    auto *reply = (redisReply *) redisCommand(context, "DEL %s", stream.c_str());
    if (reply == nullptr) {
        cerr << "deleteStream: Error: " << context->errstr << endl;
        return -1;
    }

    long result = reply->integer;
    freeReplyObject(reply);
    return result;
}


/**
 * Read a message from a group\n
 * XREADGROUP GROUP group consumer [COUNT count] [BLOCK milliseconds]   [NOACK] STREAMS key [key ...] id [id ...]
 * @param context redis context
 * @param group group name
 * @param consumer consumer name
 * @param stream stream name
 * @param block block time in milliseconds, 0 for waiting or -1 for no blocking
 * @return
 */
Redis::Response Redis::readMessageGroup(redisContext *context, const string &group, const string &consumer, const string &stream,
                          int block = -1) {
    // Read a message from a group
    // Create the XREADGROUP command
    string blockStr = block >= 0 ? "BLOCK " + to_string(block) : "";
    string xreadgroup =
            "XREADGROUP GROUP " + group + " " + consumer + " NOACK COUNT 1 " + blockStr + " STREAMS " + stream + " >";

    // convert to char*
    char *cstr = new char[xreadgroup.length() + 1];
    strcpy(cstr, xreadgroup.c_str());

    auto *reply = (redisReply *) redisCommand(context, cstr);
    delete[] cstr;
    if (reply == nullptr) {
        cerr << "readMessageGroup1: Error: " << context->errstr << endl;
        return make_tuple("", map<string, string>());
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        cerr << "readMessageGroup2: Error: " << reply->str << endl;
        freeReplyObject(reply);
        return make_tuple("", map<string, string>());
    }

    if (reply->type != REDIS_REPLY_ARRAY || reply->elements == 0) {
        //        cerr << "readMessageGroup: Error: Expected an array or empty" << endl;
        freeReplyObject(reply);
        return make_tuple("", map<string, string>());
    }
    // Get the message
    //                                 arr        stream       arr
    string messageId = reply->element[0]->element[1]->element[0]->element[0]->str;

    auto *message = reply->element[0]->element[1]->element[0]->element[1];

    map<string, string> messageMap;
    for (int i = 0; i < message->elements; i++) {
        messageMap[message->element[i]->str] = message->element[i + 1]->str;
        i++;
    }
    freeReplyObject(reply);
    // free
    return make_tuple(messageId, messageMap);
}
/**
 * Read group messages
 * @param context redis context
 *
 * @param group group name
 * @param consumer consumer name
 * @param stream stream name
 * @param block block time in milliseconds, 0 for waiting or -1 for no blocking
 * @param count number of messages to read at once, 0 for all
 * @return list of messages
 */
vector<Redis::Response> Redis::readGroupMessages(redisContext *context,
                                                 const string &group,
                                                 const string &consumer,
                                                 const string &stream,
                                                 int block,
                                                 int count) {
    // Read a message from a group
    // Create the XREADGROUP command
    string blockStr = block >= 0 ? "BLOCK " + to_string(block) : "";
    string countStr = count > 0 ? "COUNT " + to_string(count) : "";
    string xreadgroup =
            "XREADGROUP GROUP " + group + " " + consumer + " NOACK " + countStr + " " + blockStr + " STREAMS " +
            stream + " >";


    auto *reply = (redisReply *) redisCommand(context, xreadgroup.c_str());

    if (reply == nullptr) {
        cerr << "readMessageGroup1: Error: " << context->errstr << endl;
        return {};
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        cerr << "readMessageGroup2: Error: " << reply->str << endl;
        freeReplyObject(reply);
        return {};
    }

    if (reply->type != REDIS_REPLY_ARRAY || reply->elements == 0) {
        //        cerr << "readMessageGroup: Error: Expected an array or empty" << endl;
        freeReplyObject(reply);
        return {};
    }


    vector<Redis::Response> messages;

    //               arr        stream
    reply = reply->element[0]->element[1];
//    cout << "readGroupMessages: reply->elements: " << reply->elements<< endl;
    for (int i = 0; i < reply->elements; i++) {

        // Get the message
        //                              arr   msg
        string messageId = reply-> element[i]->element[0]->str;

        auto *message = reply->element[i]->element[1];

        map<string, string> messageMap;
        for (int j = 0; j < message->elements; j++) {
            messageMap[message->element[j]->str] = message->element[j + 1]->str;
            j++;
        }
        messages.emplace_back(messageId, messageMap);
    }

    freeReplyObject(reply);
    return messages;
}


/**
 * Acknowledge a message \n
 * XACK key group ID [ID ...]
 * @param context redis context
 * @param stream stream name
 * @param group group name
 * @param messageId message id
 * @return number of messages acknowledged
 */
long Redis::ackMessage(redisContext *context, const string &stream, const string &group, const string &messageId) {
    // Acknowledge a message
    auto *reply = (redisReply *) redisCommand(context, "XACK %s %s %s", stream.c_str(), group.c_str(),
                                              messageId.c_str());
    if (reply == nullptr) {
        cerr << "acknowledgeMessage: Error: " << context->errstr << endl;
        return -1;
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        cerr << "acknowledgeMessage: Error: " << reply->str << endl;
        freeReplyObject(reply);
        return -1;
    }
    long result = reply->integer;
    freeReplyObject(reply);
    return result;
}


/**
 * Get the group info from a stream
 * @param context redis context
 * @param stream stream name
 * @param nameGroup group name
 * @return group info
 */
Redis::GroupInfo Redis::getInfoGroup(redisContext *context, const string &stream, const string &nameGroup) {
    //TODO: Gestire quando il gruppo non esiste

    // Get the info
    auto *reply = (redisReply *) redisCommand(context, "XINFO GROUPS %s", stream.c_str());
    if (reply == nullptr) {
        cerr << "printInfoGroup: Error: " << context->errstr << endl;
        return {};
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        cerr << "printInfoGroup: Error: " << reply->str << endl;
        freeReplyObject(reply);
        return {};
    }

    Redis::GroupInfo groupInfo;

    // Print the info
    for (int i = 0; i < reply->elements; i++) {
        auto *group = reply->element[i];
        string name = group->element[1]->str;
        if (name != nameGroup) {
            continue;
        }

        // Build the group info
        groupInfo.name = group->element[1]->str;
        groupInfo.consumers = group->element[3]->integer;
        groupInfo.pending = group->element[5]->integer;
        groupInfo.lastDeliveredId = group->element[7]->str;
        groupInfo.entriesRead = group->element[9]->integer;
        groupInfo.lag = group->element[11]->integer;
        break;


    }
    freeReplyObject(reply);
    return groupInfo;
}

/**
 * Get the stream info
 * @param context redis context
 * @param stream stream name
 * @return stream info
 */
long Redis::getStreamLen(redisContext *context, const string &stream) {
    // Get the info
    auto *reply = (redisReply *) redisCommand(context, "XINFO STREAM %s", stream.c_str());
    if (reply == nullptr) {
        cerr << "printInfoStream: Error: " << context->errstr << endl;
        return {};
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        cerr << "printInfoStream: Error: " << reply->str << endl;
        freeReplyObject(reply);
        return {};
    }

    if (reply->type == REDIS_REPLY_ARRAY) {
        for (size_t i = 0; i < reply->elements; i += 2) {
            std::string key = reply->element[i]->str;
            redisReply *value = reply->element[i + 1];
            if (key == "length") {
                return value->integer;
            }
        }
    }
}