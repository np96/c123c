//
// Created by Николай on 01/09/16.
//

#ifndef CHAT_NONCOPYABLE_H
#define CHAT_NONCOPYABLE_H


class NonCopyable {
public:
    NonCopyable() {

    }

    virtual ~NonCopyable() {

    }

protected:

    NonCopyable(const NonCopyable &other) = delete;

    NonCopyable &operator=(const NonCopyable &other) = delete;

    NonCopyable(NonCopyable &&other) = delete;

    NonCopyable &operator=(NonCopyable &&other) = delete;

};


#endif //CHAT_NONCOPYABLE_H
