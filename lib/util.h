//
// Created by Николай on 19/02/17.
//

#ifndef CHAT_UTIL_H
#define CHAT_UTIL_H

#include <string>
#include <third_party/json.hpp>

namespace util {
    using std::string;
    using std::vector;

    string getKeyValue(const json &body, const string &key);

    bool isNumber(const string &s);

    string currentTime();
}

#endif //CHAT_UTIL_H
