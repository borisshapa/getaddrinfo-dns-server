//
// Created by borisshapa on 27.12.2019.
//

#ifndef SERVER_EVENT_HANDLER_H
#define SERVER_EVENT_HANDLER_H

#include <functional>
#include "epoll_wrapper.h"

struct event_handler {
    typedef std::function<void()> on_event_t;

    event_handler(epoll_wrapper &epoll_w, on_event_t callback);

    raii_fd &get_fd();

private:
    raii_fd event_fd;
    on_event_t callback;
    epoll_registration handler;

    int create_eventfd();
};


#endif //SERVER_EVENT_HANDLER_H
