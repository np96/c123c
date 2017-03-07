//
// Created by Николай on 22/08/16.
//

#ifndef CHAT_SERVERTHREAD_H
#define CHAT_SERVERTHREAD_H

#include <future>
#include <vector>
#include "RequestFunctions.h"
#include "Holders/SocketHolder.h"
#include "Holders/HttpEvHolder.h"
#include <unistd.h>

class ServerThread : public SocketHolder {

public:
    ServerThread(uint16_t threads = 1, uint16_t listPort = 6667);

    void Run() {
        run(n_threads);
    }

    ~ServerThread() {
        stop = true;
    }

private:


    bool stop;
    uint16_t n_threads;
    uint16_t port;

    typedef std::function<void(redisContext *, const json &, evhtp_request_t *)> Handle;

    unordered_map<string, Handle> defaultMapping = {
            {"/groups", &reqproc::groupsRequestHandle},
            {"/login",  &reqproc::loginRequestHandle},
            {"/signup", &reqproc::signUpRequestHandle},
            {"/poll", &reqproc::pollingHandle},
            {"/new_message", &reqproc::newMessageHandle},
            {"/groupmembers", &reqproc::groupMembersHandle},
            {"/add_member_to_gr", &reqproc::addMemberHandle},
            {"/new_group", &reqproc::newGroupHandle},
            {"/msg_content", &reqproc::messageContentHandle}
    };

    void run(uint16_t nthreads) {
        std::vector<std::future<void>> vect;

        for (int i = 0; i < nthreads; ++i) {
            vect.push_back(std::async(std::launch::async, &ServerThread::dispatch, this, sd, i));
            std::cout << i + 1 << std::endl;
        }
        for (auto &fut: vect) {
            fut.get();
        }
        std::cin.get();
    }


    void dispatch(SockDescr sd, int i) {
        HttpEvHolder holder = {
                "127.0.0.1", port, defaultMapping
        };

        while (!stop) {
            if (event_base_loop(holder.base(), EVLOOP_NONBLOCK)) {
                LOG(ERROR) << "DISPATCH FAILED";
            }
            usleep(10000);
        }
        std::cout << "stopped" << std::endl;
    }
};


#endif //CHAT_SERVERTHREAD_H

///
/*
 * hm users::352152 login password grouplist friendlist
 *
 * bitset usersonline
 * hash token id -> HttpSessionId
 * hm groups::124::1 privilegy messageid
 * groups::124 list
 *
 *
 */
