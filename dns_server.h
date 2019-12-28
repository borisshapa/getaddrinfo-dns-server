//
// Created by borisshapa on 27.12.2019.
//

#ifndef SERVER_DNS_SERVER_H
#define SERVER_DNS_SERVER_H

#include <cstddef>
#include <map>
#include <memory>
#include "epoll_wrapper.h"
#include "ipv4_endpoint.h"
#include "socket.h"
#include "dns_thread_pool.h"

struct dns_server {
    struct connection {
        connection(dns_server *parent, size_t id);

        void process();

        bool process_read();

        bool process_write();

    private:
        size_t id;
        dns_server *parent;
        client_socket sock;
        size_t start_offset;
        size_t end_offset;
        char buf[1024];
    };

    dns_server(epoll_wrapper &epoll_w, ipv4_endpoint const &local_endpoint);

private:
    void on_new_connection();

    epoll_wrapper &epoll_w;
    server_socket sock;
    std::map<connection *, std::unique_ptr<connection>> connections;
    std::queue<connection*> ids;
    dns_thread_pool tp;
};


#endif //SERVER_DNS_SERVER_H
