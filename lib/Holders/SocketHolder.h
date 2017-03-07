//
// Created by Николай on 15/10/16.
//

#ifndef CHAT_SOCKETHOLDER_H
#define CHAT_SOCKETHOLDER_H


#include <cstdint>
#include <sys/socket.h>
#include <fcntl.h>
#include "NonCopyable.h"
#include "third_party/easylogging++.h"
class SocketHolder: public virtual NonCopyable {

private:
    void initSocket();

protected:
    uint16_t port;
    int sd;
public:

    ~SocketHolder() {
        shutdown(sd, SHUT_RD);
    }

    SocketHolder(uint16_t sock_port): port(sock_port) {
        LOG(INFO) << "initing sock " << sock_port;
        //initSocket();
    }


    static int
    setnonblock(int fd)
    {
        int flags;

        flags = fcntl(fd, F_GETFL);
        if (flags < 0)
            return flags;
        flags |= O_NONBLOCK;
        if (fcntl(fd, F_SETFL, flags) < 0)
            return -1;

        return 0;
    }

};


#endif //CHAT_SOCKETHOLDER_H
