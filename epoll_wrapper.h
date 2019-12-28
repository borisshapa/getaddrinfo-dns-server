//
// Created by borisshapa on 20.12.2019.
//

#ifndef SERVER_EPOLL_WRAPPER_H
#define SERVER_EPOLL_WRAPPER_H


#include <cstdint>
#include <functional>
#include "raii_fd.h"
#include "timer.h"

const int MAX_EVENTS_COUNT = 128;

struct epoll_wrapper;
struct epoll_registration;

struct epoll_wrapper {
    epoll_wrapper();

    ~epoll_wrapper() = default;

    void run();

    timer& get_timer();
private:
    void epw_ctl(int action, int fd, epoll_registration *event);

    void add(int fd, epoll_registration *event);

    void modify(int fd, epoll_registration *event);

    void remove(int fd);

    int run_timers_calculate_timeout();

    raii_fd epoll_fd;

    timer timer_;

    bool stopped;

    friend struct epoll_registration;
};

struct epoll_registration {
    typedef std::function<void(uint32_t)> callback_t;

    epoll_registration(epoll_wrapper &epoll_w, int fd, uint32_t events, callback_t callback);

    ~epoll_registration();

    void modify(uint32_t new_events);

    epoll_wrapper &get_epoll() const;


private:
    epoll_wrapper *epoll_w;
    int fd;
    uint32_t events;
    callback_t callback;
    friend struct epoll_wrapper;
};

#endif //SERVER_EPOLL_WRAPPER_H
