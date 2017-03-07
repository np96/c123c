//
// Created by Николай on 15/10/16.
//

#include <netinet/in.h>
#include <third_party/easylogging++.h>
#include "SocketHolder.h"
#include "HolderInitException.h"


void SocketHolder::initSocket() {
    sockaddr_in addr = {0};
    addr.sin_family = PF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    sd = socket(PF_INET, SOCK_STREAM, 0);
    int temp = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &temp, sizeof(int)) < 0) {
        LOG(ERROR) << "SO_REUSEADDR ERROR";
        throw HolderInitException("reuseaddr");
    }
    if (sd < 0) {
        LOG(ERROR) << "SOCK CREATION ERROR " << errno;
        throw HolderInitException("socket create");
    }
    if (bind(sd, (sockaddr *) &addr, sizeof(addr)) < 0) {
        LOG(ERROR) << "BIND ERROR " << errno;
        throw HolderInitException("socket bind");
    }
    if (listen(sd, 512) < 0) {
        LOG(ERROR) << "LISTEN ERROR " << errno;
        throw HolderInitException("socket listen");
    }
    if (setnonblock(sd)) {
        LOG(ERROR) << "NONBLOCK ERROR";
        throw HolderInitException("nonblock");
    }
}