//
// Created by Николай on 27/09/16.
//

#ifndef CHAT_REQUESTHANDLER_H
#define CHAT_REQUESTHANDLER_H


#include <hiredis/hiredis.h>
#include <evhttp.h>
#include <third_party/easylogging++.h>
#include <evhtp.h>
#include "third_party/json.hpp"
#include "MessageParser.h"

using json=nlohmann::json;

class RequestHandler {


public:

    typedef std::function<void(redisContext *, const json &, evhtp_request_t *)> Handle;

    RequestHandler(redisContext *rc, Handle hand) : redContext(rc), handler(hand) {


    }

    static void abstractRequestHandle(evhtp_request_t *request, void *raw) {
        //LOG(INFO) << "handling request: " << request->uri;
        try {
            auto handl = static_cast<RequestHandler *>(raw);
            handl->handler(handl->redContext, json::parse(MessageParser::getRequestBody(request)), request);

        } catch (...) {
            sendErrorReply(request);
        }
    }


private:

    static void sendErrorReply(evhtp_request_t *request) {
        LOG(INFO) << "Error on request: " << MessageParser::getRequestBody(request);

        evbuffer_add_printf(request->buffer_out, "Error");
        evhtp_send_reply(request, EVHTP_RES_OK);
    }

    Handle handler;
    redisContext *redContext;
};

#endif //CHAT_REQUESTHANDLER_H

/*
 * static void MessagesRequestHandle(evhttp_request *request, void *data) {
        LOG(INFO) << "handling request:\n" << evhttp_request_get_connection(request);
        auto hld = reinterpret_cast<HttpEvHolder *>(data);
        json body;
        try {
            body = json::parse(MessageParser::getRequestBody(request));
        } catch (...) {
            hld->sendErrorReply(request);
            return;
        }
        RedisCommands::getAllAvailableGroups(hld->redContext(), )
    }
 */