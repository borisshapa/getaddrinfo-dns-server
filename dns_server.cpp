//
// Created by borisshapa on 27.12.2019.
//

#include "dns_server.h"

dns_server::connection::connection(dns_server *parent, size_t id)
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
        , buf()
        , id(id) {}

bool dns_server::connection::process_read() {
    end_offset = sock.recv(buf, sizeof(buf));
    bool ret = true;

    for (size_t i = 0; i < end_offset - 1; i++) {
        if (std::string(buf + i, buf + i + 2) == "\r\n") {
            std::string hostname(buf, buf + i);
            parent->tp.resolve(hostname, id, [this, &ret] {
                sock.set_on_read_write({}, [this] { process_write(); });
                ret = false;
            });
        }
    }

    if (end_offset == 0) {
        sock.set_on_read_write([this] { process(); }, {});
        ret = false;
    }
    return ret;
}

bool dns_server::connection::process_write() {
    bool ret = true;
    std::string res = parent->tp.get_response(id);
    int count = sock.send(const_cast<char *>(res.c_str()), res.size());

    if (res.size() != count) {
        sock.set_on_read_write({}, [this] { process_write(); });
        ret = false;
    }

    if (end_offset != sizeof(buf)) {
        sock.set_on_read_write([this] { process(); }, {});
        ret = false;
    }

    return ret;
}

void dns_server::connection::process() {
    while (true) {
        if (!process_read() || !process_write()) {
            return;
        }
    }
}

dns_server::dns_server(epoll_wrapper &epoll_w, ipv4_endpoint const &local_endpoint)
        : epoll_w(epoll_w)
        , sock(epoll_w, local_endpoint, std::bind(&dns_server::on_new_connection, this))
        , tp(4) {}

void dns_server::on_new_connection() {
    std::unique_ptr<connection> new_conn(new connection(this, ids.size()));
    connections.emplace(new_conn.get(), std::move(new_conn));
    ids.push(new_conn.get());
}
