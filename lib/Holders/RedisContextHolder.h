//
// Created by Николай on 16/10/16.
//

#ifndef CHAT_REDISCONTEXTHOLDER_H
#define CHAT_REDISCONTEXTHOLDER_H


#include <hiredis/hiredis.h>
#include <NonCopyable.h>
#include <third_party/easylogging++.h>
#include "HolderInitException.h"

class RedisContextHolder : virtual NonCopyable {
public:


    RedisContextHolder(const char *redisIp = "127.0.0.1", int redisPort = 6379) : redis_context(
            redisConnect(redisIp, redisPort)) {
        if (redis_context == nullptr || redis_context->err) {
            LOG(ERROR) << "Error: " << (redis_context ? redis_context->errstr : "Failed to connect to redis");
            throw HolderInitException("redis connect");
        }
    }

    redisContext *redContext() const noexcept {
        return redis_context;
    }

    ~RedisContextHolder() {
        redisFree(redis_context);
    }

private:

    redisContext *redis_context;


};


#endif //CHAT_REDISCONTEXTHOLDER_H
