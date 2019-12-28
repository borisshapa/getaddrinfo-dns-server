//
// Created by borisshapa on 27.12.2019.
//

#include <sys/epoll.h>
#include "event_handler.h"
#include <sys/eventfd.h>
#include <iostream>

event_handler::event_handler(epoll_wrapper &epoll_w, event_handler::on_event_t callback)
        : event_fd(create_eventfd()), callback(std::move(callback)),
          handler(epoll_w, event_fd.get_fd(), EPOLLIN, [this](uint32_t) {
              uint64_t tmp;
              while (event_fd.recv(&tmp, sizeof(tmp)) != -1) {
                  this->callback();
              }
          }) {}

int event_handler::create_eventfd() {
    int res = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC | EFD_SEMAPHORE);
    if (res == -1) {
        std::cerr << "Can't create event_fd";
    }
    return res;
}

raii_fd& event_handler::get_fd() {
    return event_fd;
}