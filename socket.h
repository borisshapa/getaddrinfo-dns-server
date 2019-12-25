//
// Created by borisshapa on 20.12.2019.
//

#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#include <sys/epoll.h>
#include <netinet/in.h>
#include <functional>
#include <iostream>
#include "epoll_wrapper.h"
#include "ipv4_endpoint.h"

struct socket {
    socket(raii_fd &&fd);

    socket(int fd);

    ~socket() = default;

    static int create_socket(int domain, int type);

    void bind_listen(uint32_t address, uint16_t port);

    int get_fd() const;

    ssize_t recv(void *buf, size_t count);

    ssize_t send(void *buf, size_t count);

private:
    raii_fd fd;
};


struct client_socket : public socket {
    typedef std::function<void()> on_ready_t;

    client_socket(epoll_wrapper &epoll_w, raii_fd &&fd, on_ready_t on_disconnect, on_ready_t on_read_ready,
                  on_ready_t on_write_ready);

    ~client_socket();

    void set_on_read(on_ready_t on_read_ready);

    void set_on_write(on_ready_t on_write_ready);

    void set_on_read_write(on_ready_t on_read_ready, on_ready_t on_write_ready);

    int calculate_flags() const;

private:
    on_ready_t on_disconnect;
    on_ready_t on_read_ready;
    on_ready_t on_write_ready;
    epoll_registration ep_reg;
    bool *destroyed;
};


struct server_socket : public socket {
    typedef std::function<void()> on_connected_t;

    server_socket(epoll_wrapper &epoll_w, ipv4_endpoint local_endpoint, on_connected_t on_connected);

    ~server_socket() = default;

    client_socket accept(client_socket::on_ready_t on_disconnect,
                         client_socket::on_ready_t on_read_ready,
                         client_socket::on_ready_t on_write_ready) const;

private:
    on_connected_t on_connected;
    epoll_registration ep_reg;
};


#endif //SERVER_SOCKET_H
