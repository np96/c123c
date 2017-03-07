//
// Created by Николай on 19/09/16.
//

#ifndef CHAT_LOGINPASSWORDPAIR_H
#define CHAT_LOGINPASSWORDPAIR_H
//
// Created by Николай on 19/09/16.
//

#include "third_party/easylogging++.h"
#include "RedisCommands.h"
#include "LoginPasswordPair.h"

using std::string;

class LoginPasswordPair {

public:
    const string &getPassword() const {
        return password;
    }

    const string &getLogin() const {
        return login;
    }

    LoginPasswordPair(const string &login, const string &password) {
        initialize(login, password);
    }


    LoginPasswordPair(const json &body) {

        auto itLogin = body.find("login");
        auto itPassword = body.find("password");
        if (itLogin == body.end() || itPassword == body.end()) {
            initialize("", "");
            return;
        }

        auto str_login = itLogin.value().dump();
        auto str_password = itPassword.value().dump();
        if (str_login.size() < 3 || str_password.size() < 3) {
            initialize("", "");
            return;
        }
        initialize(str_login.substr(1, str_login.size() - 2),
                   str_password.substr(1, str_password.size() - 2));

    }

    operator bool() const {
        return isValid();
    }

    bool isValid() const {
        return login != "" && password != "";
    }

private:

    LoginPasswordPair() {

    }

    void initialize(const string &login, const string &password) {
        this->login = login;
        this->password = password;
    }

    string login;
    string password;

};

#endif //CHAT_LOGINPASSWORDPAIR_H