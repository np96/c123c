//
// Created by Николай on 12/08/16.
//

#ifndef CHAT_MESSAGEPARSER_H
#define CHAT_MESSAGEPARSER_H


#include <string>
#include <evhtp.h>
#include <map>

using std::map;

namespace MessageParser {
    //std::string parse_message(std::string mes);

    std::string getRequestBody(evhtp_request_t * request);
};

#endif //CHAT_MESSAGEPARSER_H
