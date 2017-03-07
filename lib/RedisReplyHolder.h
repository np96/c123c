//
// Created by Николай on 01/09/16.
//



#ifndef CHAT_REDISREPLYHOLDER_H
#define CHAT_REDISREPLYHOLDER_H

#endif //CHAT_REDISREPLYHOLDER_H

#include <hiredis/hiredis.h>
#include "NonCopyable.h"
#include <string>
#include "third_party/json.hpp"
#include <vector>
#include <new>
#include <memory>

using std::vector;
using std::string;
using nlohmann::json;

class RedisReplyHolder : NonCopyable {
private:
    redisReply *reply;

    mutable json jsonResult;

public:

    int type() const {
        return reply->type;
    }

    redisReply *getReply() const {
        return reply;
    }

    long long int integer() const {
        return reply->integer;

    }

    RedisReplyHolder(void *command) : reply(reinterpret_cast<redisReply *>(command)) {

    }

    ~RedisReplyHolder() {
        freeReplyObject(reply);
    }

    RedisReplyHolder(redisContext *context, const char *format, ...) {
        char buf[80];           //todo: fix this
        va_list argptr;
        va_start(argptr, format);
        vsprintf(buf, format, argptr);
        va_end(argptr);
        reply = reinterpret_cast<redisReply *>(redisCommand(context, buf));
    }

    inline redisReply **arrayReply() const {
        return reply->element;
    }

    inline size_t replySize() const {
        return reply->elements;
    }


    vector<string> getReplyArray() const {
        vector<string> res = {};
        for (int i = 0; i < replySize(); ++i) {
            res.emplace_back(arrayReply()[i]->str, arrayReply()[i]->len);
        }
        return res;
    }

    json getJsonReply() const {
        if (jsonResult.size()) return jsonResult;
        const auto &res = getReplyArray();
        for (int i = 0; i < res.size(); i += 2) {
            jsonResult[res[i]] = res[i + 1];
        }
        return jsonResult;
    }

    RedisReplyHolder(redisContext *context, const vector<string> &args) {
        string req;
        for (const auto &elem: args) req += elem;
        reply = reinterpret_cast<redisReply *>(redisCommand(context, req.c_str()));
    }

    static std::unique_ptr<RedisReplyHolder> messageCommand(redisContext *redContext, const vector<string> &arguments) {
        const char *formatString = "HMSET messages::%s content %s timestamp %s author %s group %s";
        return std::unique_ptr<RedisReplyHolder>(new RedisReplyHolder(redisCommand(redContext, formatString, arguments[0].c_str(),
                                                                               arguments[1].c_str(),
                                                                               arguments[2].c_str(),
                                                                               arguments[3].c_str(),
                                                                               arguments[4].c_str())));
    }

};