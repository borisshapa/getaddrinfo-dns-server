//
// Created by borisshapa on 27.12.2019.
//

#include "dns_server.h"

namespace
{
    constexpr const timer::clock_t::duration timeout = std::chrono::seconds(15);
}

dns_server::connection::connection(dns_server *parent)
        : parent(parent)
        , sock(parent->sock.accept(
                // on_disconnect
                [this, parent] { parent->connections.erase(this); },
                // on_read
                [this] { process(); },
                // on_write
                {}))
        , end_offset()
        , buf()
        , id(parent->hash_fn(this))
        , timer_elem(parent->epoll_w.get_timer(), timeout, [this] {
            this->parent->connections.erase(this);
        }){}

void dns_server::connection::disconnect() {
    parent->connections.erase(this);
}

bool dns_server::connection::process_read() {
    size_t count = sock.recv(buf, sizeof(buf));

    size_t start_offset = cache.size();
    cache += std::string(buf, buf + count);
    if (cache.size() > 64 * MB) {
        disconnect();
    }

    size_t offset = 0;
    start_offset = (start_offset == 0) ? 0 : start_offset - 1;
    for (size_t i = start_offset; i < cache.size() - 1; i++) {
        std::string window = cache.substr(i, 2);
        if (window == "\r\n") {
            std::string hostname(cache.data() + offset, cache.data() + i);
            parent->tp.resolve(hostname, id, [this] {
                sock.set_on_read_write({}, [this] { process_write(); });
            });
            offset = i + 2;
        }
    }

    if (offset != cache.size()) {
        cache = cache.substr(offset);
    } else {
        cache.clear();
    }

    if (end_offset == 0) {
        sock.set_on_read_write([this] { process(); }, {});
        return false;
    }
    return true;
}

bool dns_server::connection::process_write() {
    cache_w += parent->tp.get_response(id);
    int count = sock.send(const_cast<char *>(cache_w.c_str()), cache_w.size());

    if (cache_w.size() > 64 * MB) {
        disconnect();
    }

    if (count > 0) {
        timer_elem.restart(parent->epoll_w.get_timer(), timeout);
    }

    if (count < cache_w.size()) {
        cache_w.substr(count);
        sock.set_on_read_write({}, [this] { process_write(); });
        return false;
    } else {
        cache_w.clear();
    }

    if (end_offset != sizeof(buf)) {
        sock.set_on_read_write([this] { process(); }, {});
        return false;
    }

    return true;
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
    std::unique_ptr<connection> new_conn(new connection(this));
    connections.emplace(new_conn.get(), std::move(new_conn));
}
