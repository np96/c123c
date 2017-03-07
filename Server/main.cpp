#define ELPP_THREAD_SAFE
//must be defined before including easylogging
//TODO: to -DELPP_THREADSAFE
#include <iostream>
#include "../lib/third_party/easylogging++.h"
#include "../lib/ServerThread.h"

//must be done only once before logger access
INITIALIZE_EASYLOGGINGPP

int main(int argc, const char **argv) {

    //ignore sigpipe, handle socket write errors in application
    signal(SIGPIPE, SIG_IGN);
    el::Configurations defaultConf;
    defaultConf.setToDefault();
    defaultConf.setGlobally(el::ConfigurationType::Format, "%datetime %level %msg");
    el::Loggers::reconfigureLogger("default", defaultConf);
    LOG(INFO) << "EXECUTING";
    ServerThread threadMaster{};
    threadMaster.Run();
}

