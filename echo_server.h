//
// Created by borisshapa on 24.12.2019.
//

#ifndef SERVER_ECHO_SERVER_H
#define SERVER_ECHO_SERVER_H


#include <cstddef>
#include <map>
#include <memory>
#include "epoll_wrapper.h"
#include "ipv4_endpoint.h"
#include "socket.h"

struct echo_server {
    struct connection {
        connection(echo_server *parent);

        void process();

        bool process_read();

        bool process_write();

    private:
        echo_server *parent;
        client_socket sock;
        size_t start_offset;
        size_t end_offset;
        char buf[1024];
    };

    echo_server(epoll_wrapper &epoll_w, ipv4_endpoint const &local_endpoint);

private:
    void on_new_connection();

    epoll_wrapper &epoll_w;
    server_socket sock;
    std::map<connection *, std::unique_ptr<connection>> connections;
};


#endif //SERVER_ECHO_SERVER_H
