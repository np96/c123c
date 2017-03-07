//
// Created by Николай on 12/08/16.
//

#include <cstdlib>
#include "MessageParser.h"
#include "third_party/json.hpp"

using json = nlohmann::json;


std::string MessageParser::getRequestBody(evhtp_request_t *request) {
    evbuffer *buf = request->buffer_in;
    size_t len = evbuffer_get_length(buf);
    char temp[len];
    evbuffer_copyout(buf, temp, len);
    std::string body(temp, len);
    return body;
}
