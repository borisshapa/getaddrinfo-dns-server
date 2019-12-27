//
// Created by borisshapa on 20.12.2019.
//

#include "socket.h"
#include <sys/socket.h>
#include <iostream>
#include <utility>

socket::socket() {
    int res = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    std::cerr << res << std::endl;
    if (res == -1) {
        std::cerr << "Can't create socket." << std::endl;
    }
    fd = res;
}

socket::socket(raii_fd &&fd)
        : fd(std::move(fd)) {}

socket::socket(int fd)
    : fd(fd) {}

int socket::get_fd() const {
    return fd.get_fd();
}

ssize_t socket::recv(void *buf, size_t count) {
    return ::recv(fd.get_fd(), buf, count, 0);
}

ssize_t socket::send(void *buf, size_t count) {
    return ::send(fd.get_fd(), buf, count, 0);
}

server_socket::server_socket(epoll_wrapper &epoll_w, ipv4_endpoint local_endpoint,
                             server_socket::on_connected_t on_connected)
        : socket()
        , on_connected(std::move(on_connected))
        , ep_reg(epoll_w, get_fd(), EPOLLIN, [this](uint32_t) {
            this->on_connected();
        }) {
    bind_listen(local_endpoint.address(), local_endpoint.port());
}

void server_socket::bind_listen(uint32_t address, uint16_t port) {
    sockaddr_in saddr{};
    saddr.sin_family = AF_INET;
    saddr.sin_port = port;
    saddr.sin_addr.s_addr = address;

    int opt = 1;
    int res = ::setsockopt(get_fd(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);
    if (res < 0) {
        std::cerr << "set sockopt";
    }

    res = ::bind(get_fd(), reinterpret_cast<sockaddr const *>(&saddr), sizeof(saddr));
    if (res == -1) {
        std::cerr << "Error during bind." << std::endl;
    }

    res = ::listen(get_fd(), SOMAXCONN);
    if (res == -1) {
        std::cerr << "Error during listen.";
    }
}

client_socket server_socket::accept(client_socket::on_ready_t on_disconnect, client_socket::on_ready_t on_read_ready,
                                    client_socket::on_ready_t on_write_ready) const {
    int res = ::accept(get_fd(), nullptr, nullptr);
    std::cerr << res << std::endl;
    if (res == -1) {
        std::cerr << "Error in accept." << std::endl;
    }
    return client_socket(ep_reg.get_epoll(),
                         std::move(raii_fd(res)),
                         std::move(on_disconnect),
                         std::move(on_read_ready),
                         std::move(on_write_ready));
}

client_socket::client_socket(epoll_wrapper &epoll_w,
                            raii_fd &&fd,
                            on_ready_t on_disconnect,
                            on_ready_t on_read_ready,
                            on_ready_t on_write_ready)
        : socket(std::move(fd))
        , on_disconnect(std::move(on_disconnect))
        , on_read_ready(std::move(on_read_ready))
        , on_write_ready(std::move(on_write_ready))
        , ep_reg(epoll_w,
                get_fd(),
                calculate_flags(),
                // TODO: WHY IS DESTROED?
                [this](uint32_t events) {
                    bool is_destroed = false;
                    destroyed = &is_destroed;
                    try {
                        if ((events & EPOLLRDHUP) || (events & EPOLLERR) || (events & EPOLLHUP)) {
                            this->on_disconnect();
                            if (is_destroed) {
                                return;
                            }
                        }

                        if (events & EPOLLIN) {
                            this->on_read_ready();
                            if (is_destroed) {
                                return;
                            }
                        }

                        if (events & EPOLLOUT) {
                            this->on_write_ready();
                            if (is_destroed) {
                                return;
                            }
                        }
                    } catch (...) {
                        destroyed = nullptr;
                        throw;
                    }
                    destroyed = nullptr;
                }), destroyed(nullptr) {}


void client_socket::set_on_read_write(on_ready_t new_on_read_ready,
                                      on_ready_t new_on_write_ready) {
    on_read_ready = std::move(new_on_read_ready);
    on_write_ready = std::move(new_on_write_ready);
    ep_reg.modify(calculate_flags());
}

int client_socket::calculate_flags() const {
    return EPOLLRDHUP
           | (on_read_ready ? EPOLLIN : 0)
           | (on_write_ready ? EPOLLOUT : 0);
}

client_socket::~client_socket() {
    if (destroyed)
        *destroyed = true;
}
