#include "json.hpp"
#include <hiredis/hiredis.h>

using json = nlohmann::json;


json redisReplyToJSON(redisReply *reply);
void deleteMessage(const std::string& stream, const std::string& messageId, redisContext *ctx);
void createGroup(redisContext *ctx, const std::string& stream, const std::string& group);