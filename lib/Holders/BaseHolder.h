

#ifndef CHAT_BASESERVER_H
#define CHAT_BASESERVER_H

#include <event.h>
#include "Holders/HolderInitException.h"
#include "NonCopyable.h"
#include <evhtp.h>

class BaseHolder: public virtual NonCopyable {

public:
    BaseHolder() : evBase(event_base_new()) {
        if (!evBase) throw HolderInitException("BaseHolder");
    }


    virtual ~BaseHolder() {
        event_base_free(evBase);
    }

    event_base *base() const noexcept {
        return evBase;
    }

protected:

private:


    evbase_t *evBase;
};


#endif //CHAT_BASESERVER_H
