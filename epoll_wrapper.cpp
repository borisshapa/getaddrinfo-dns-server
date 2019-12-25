//
// Created by borisshapa on 20.12.2019.
//

#include <sys/epoll.h>
#include <iostream>
#include <unistd.h>
#include <csignal>
#include "epoll_wrapper.h"

epoll_wrapper::epoll_wrapper()
        : stopped(false)
        , epoll_fd(epoll_create(1)) {
    if (!epoll_fd.valid_existing()) {
        std::cerr << "Can't create epoll file descriptor." << std::endl;
    }
}

void epoll_wrapper::run() {
    raii_fd sig_fd = raii_fd::signal_fd({SIGINT, SIGTERM});
    epoll_registration sig_event(*this, sig_fd.get_fd(), EPOLLIN, [this](uint32_t) {
        std::cerr << "Signal caught." << std::endl;
        stopped = true;
    });

    epoll_event events[MAX_EVENTS_COUNT];
    while (!stopped) {
        int res = epoll_wait(epoll_fd.get_fd(), events, MAX_EVENTS_COUNT, -1);
        if (res == -1) {
            // TODO: What is it?
            if (errno != EINTR) {
                std::cerr << "Error in epoll_wait.";
            } else {
                break;
            }
        } else {
            for (size_t i = 0; i < res; i++) {
                auto const & event = events[i];
                try {
                    static_cast<epoll_registration *>(event.data.ptr)->callback(event.events);
                } catch (std::exception const &e) {
                    std::cerr << "Error: " << e.what() << std::endl;
                }
            }
        }
    }
}

void epoll_wrapper::epw_ctl(int action, int fd, epoll_registration *event,  uint32_t events) {
    epoll_event ep_event = {0, nullptr};
    ep_event.data.ptr = event;
    ep_event.events = events;

    int res = epoll_ctl(epoll_fd.get_fd(), action, fd, &ep_event);
    if (res == -1) {
        std::cerr << "Error in epoll_ctl: " + std::to_string(action) << std::endl;
    }
}

void epoll_wrapper::add(int fd, epoll_registration *event, uint32_t events) {
    epw_ctl(EPOLL_CTL_ADD, fd, event, events);
};

void epoll_wrapper::remove(int fd) {
    epw_ctl(EPOLL_CTL_DEL, fd, nullptr, 0);
}

void epoll_wrapper::modify(int fd, epoll_registration *event, uint32_t events) {
    epw_ctl(EPOLL_CTL_MOD, fd, event, events);
}

epoll_registration::epoll_registration(epoll_wrapper &epoll_w, int fd, uint32_t events,
                                       epoll_registration::callback_t callback)
                                       : epoll_w(&epoll_w)
                                       // TODO: CHECK
                                       , fd(fd)
                                       , events(events)
                                       , callback(std::move(callback)) {
    epoll_w.add(fd, this, events);
}

epoll_wrapper &epoll_registration::get_epoll() const {
    return *epoll_w;
}

void epoll_registration::modify(uint32_t new_events) {
    if (events == new_events)
        return;

    epoll_w->modify(fd, this, new_events);
    events = new_events;
}

epoll_registration::~epoll_registration() {
    if (epoll_w) {
        epoll_w->remove(fd);
        epoll_w = nullptr;
        fd = -1;
        events = 0;
    }
}
