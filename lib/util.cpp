//
// Created by Николай on 19/02/17.
//
#include <string>
#include <third_party/json.hpp>
#include <algorithm>
#include <chrono>

namespace util {
    using std::string;
    using std::vector;
    using std::lock_guard;
    using std::mutex;
    using std::pair;
    using std::chrono::system_clock;
    using nlohmann::json;


    string getKeyValue(const json &body, const string &key) {
        auto it = body.find(key);
        return it != body.end() ? it.value() : "";
    }

    bool isNumber(const string &s) {
        return !s.empty() && std::find_if(s.begin(),
                                          s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
    }

    string currentTime() {
        return std::to_string(system_clock::now().time_since_epoch().count());
    }
}