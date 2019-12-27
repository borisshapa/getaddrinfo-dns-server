//
// Created by borisshapa on 24.12.2019.
//

#include <iostream>
#include "echo_server.h"

echo_server::connection::connection(echo_server *parent)
        : parent(parent)
        , sock(parent->sock.accept(
        // on_disconnect
        [this, parent] { parent->connections.erase(this); },
        // on_read
        [this] { process(); },
        // on_write
        {}))
        , start_offset()
        , end_offset()
        , buf() {}

bool echo_server::connection::process_read() {
    end_offset = sock.recv(buf, sizeof(buf));

    if (end_offset == 0) {
        sock.set_on_read_write([this] { process(); }, {});
        return false;
    }
    return true;
}

bool echo_server::connection::process_write() {
    start_offset += sock.send(buf + start_offset, end_offset - start_offset);
    if (start_offset == end_offset) {
        start_offset = end_offset = 0;
    }

    if (end_offset != 0) {
        sock.set_on_read_write({}, [this] { process_write(); });
        return false;
    }

    if (end_offset != sizeof(buf)) {
        sock.set_on_read_write([this] { process(); }, {});
        return false;
    }

    return true;
}

void echo_server::connection::process() {
    while (true) {
        if (!process_read() || !process_write()) {
            return;
        }
    }
}

echo_server::echo_server(epoll_wrapper &epoll_w, ipv4_endpoint const &local_endpoint)
        : epoll_w(epoll_w)
        , sock(epoll_w, local_endpoint, std::bind(&echo_server::on_new_connection, this)) {}

void echo_server::on_new_connection() {
    std::unique_ptr<connection> new_conn(new connection(this));
    connections.emplace(new_conn.get(), std::move(new_conn));
}
