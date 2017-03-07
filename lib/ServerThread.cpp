//
// Created by Николай on 22/08/16.
//

#include "ServerThread.h"
 //currently 1 thread only
ServerThread::ServerThread(uint16_t nthreads, uint16_t list_port) : stop(0), n_threads(1), port(list_port),
                                                                    SocketHolder(list_port) {
}

