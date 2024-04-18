#include "json.hpp"
#include <hiredis/hiredis.h>

using json = nlohmann::json;


json redisReplyToJSON(redisReply *reply);
