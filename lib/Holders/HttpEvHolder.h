//
// Created by Николай on 15/08/16.
//

#ifndef CHAT_PORTSERVER_H
#define CHAT_PORTSERVER_H

#include <fcntl.h>
#include "Holders/BaseHolder.h"
#include <event2/http.h>
#include "third_party/easylogging++.h"
#include "Holders/HolderInitException.h"
#include "MessageParser.h"
#include "RedisCommands.h"
#include <string>
#include <unordered_map>
#include <map>
#include <memory>
#include <functional>
#include <hiredis/adapters/libevent.h>
#include <hiredis/async.h>
#include "third_party/json.hpp"
#include "RequestHandler.h"
#include "RedisContextHolder.h"
#include <event2/event.h>


typedef int32_t SockDescr;
using nlohmann::json;
using std::unordered_map;
using namespace std::placeholders;


class HttpEvHolder : public BaseHolder, public RedisContextHolder {

public:

    using strToCbMap = unordered_map<string, RequestHandler::Handle>;
    typedef std::function<void(redisContext *, const json &, evhtp_request_t *)> Handle;




    bool setCallback(const std::string &url, Handle handler) {
        handlers_.emplace_back(new RequestHandler(redContext(), handler));
        return evhtp_set_cb
                       (http_base, url.c_str(), &RequestHandler::abstractRequestHandle,
                        (void *) handlers_.back().get())
               != NULL;
    }

    bool setCallbacks(const unordered_map<string, Handle> &callbacks) {
        bool res = true;
        for (auto callback: callbacks) {
            if (!setCallback(callback.first, callback.second)) {
                res = false;
                LOG(INFO) << "FAILED TO SET CALLBACK FOR " << callback.first;
            }
        }
        return res;
    }

    HttpEvHolder(const char *listenIp,
                 uint16_t listenPort,
                 const strToCbMap &callbacks,
                 const char *redisIp = "127.0.0.1", int redisPort = 6379)
            :
            RedisContextHolder(redisIp, redisPort),
            http_base(evhtp_new(base(), NULL)) {

        setCallbacks(callbacks);
        if (evhtp_bind_socket(http_base, listenIp, listenPort, 512) == -1) {
            LOG(ERROR) << "HttpEventHolder failed to accept socket";
            throw HolderInitException("HttpEvHolder accept");
        }
    }

    ~HttpEvHolder() {
        evhtp_free(http_base);
    }

private:

    vector<std::unique_ptr<RequestHandler>> handlers_{};

    evhtp_t *http_base;
};

#endif //CHAT_PORTSERVER_H
