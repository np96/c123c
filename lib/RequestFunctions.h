//
// Created by Николай on 10/10/16.
//

#ifndef CHAT_REQUESTFUNCTIONS_H
#define CHAT_REQUESTFUNCTIONS_H

#include <unordered_map>
#include <hiredis/hiredis.h>
#include <string>
#include <evhtp.h>
#include "third_party/json.hpp"

using json=nlohmann::json;

namespace reqproc {

    void newGroupHandle(redisContext *, const json &, evhtp_request_t *);

    void pollingHandle(redisContext *, const json &, evhtp_request_t *);

    void newMessageHandle(redisContext *, const json &, evhtp_request_t *);

    void groupsRequestHandle(redisContext *, const json &, evhtp_request_t *);

    void loginRequestHandle(redisContext *, const json &, evhtp_request_t *);

    void signUpRequestHandle(redisContext *, const json &, evhtp_request_t *);

    void groupMembersHandle(redisContext *, const json &, evhtp_request_t *);

    void addMemberHandle(redisContext *, const json &, evhtp_request_t *);

    void messageContentHandle(redisContext *, const json &, evhtp_request_t *);

}


#endif //CHAT_REQUESTFUNCTIONS_H

