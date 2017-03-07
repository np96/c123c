//
// Created by Николай on 20/08/16.
//

#ifndef CHAT_HOLDERINITEXCEPTION_H
#define CHAT_HOLDERINITEXCEPTION_H

#include <exception>

struct HolderInitException : public std::exception {
public:
    HolderInitException(const char *_msg) : msg(_msg) { };

    const char *what() const noexcept {
        return msg;
    }

private:
    const char *msg;


};


#endif //CHAT_HOLDERINITEXCEPTION_H
