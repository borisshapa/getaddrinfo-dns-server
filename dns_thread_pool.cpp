//
// Created by borisshapa on 27.12.2019.
//

#include "dns_thread_pool.h"

#include <utility>
#include <netdb.h>
#include <strings.h>
#include <iostream>
#include <arpa/inet.h>
#include <csignal>

dns_thread_pool::dns_thread_pool(size_t threads_count)
        : finished(false) {
    for (size_t i = 0; i < threads_count; i++) {
        resolvers.emplace_back(&dns_thread_pool::resolver, this);
    }
}

dns_thread_pool::~dns_thread_pool() {
    finished = true;
    for (auto &thread : resolvers) {
        kill(thread.native_handle(), SIGTERM);
    }
    cv.notify_all();

    for (auto &thread : resolvers) {
        thread.join();
    }
}

dns_thread_pool::request::request(std::string hostname, size_t id, callback_t callback)
        : hostname(std::move(hostname)), id(id), callback(std::move(callback)) {}

void dns_thread_pool::resolve(std::string const &hostname, size_t id, callback_t callback) {
    std::unique_lock<std::mutex> lg(m_req);
    req_q.push(new request(hostname, id, std::move(callback)));
    req_counts[id]++;
    cv.notify_one();
}

std::string dns_thread_pool::get_response(size_t id) {
    std::unique_lock<std::mutex> lg(m_resp);
    std::string res;
    for (auto const &address : resp[id]) {
        res += std::string(inet_ntoa(address)) + "\n";
    }
    resp[id].clear();
    return res;
}

void dns_thread_pool::resolver() {
    while (!finished) {
        request *request = nullptr;
        {
            std::unique_lock<std::mutex> lg(m_req);
            cv.wait(lg, [&] { return (!req_q.empty() || finished); });
            if (finished) {
                break;
            }

            request = req_q.front();
            req_q.pop();
        }

        std::string hostname = request->hostname;
        std::string port = "80";

        auto pos = hostname.find(':');
        if (pos != std::string::npos) {
            port = hostname.substr(pos + 1);
            hostname = hostname.substr(0, pos);
        }

        struct addrinfo hints{}, *result;
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        int res = ::getaddrinfo(hostname.c_str(), port.c_str(), &hints, &result);
        if (res == 0) {
            std::unique_lock<std::mutex> result_lg(m_resp);
            for (addrinfo *ai = result; ai != nullptr; ai = ai->ai_next) {
                resp[request->id].push_back(reinterpret_cast<sockaddr_in *>(ai->ai_addr)->sin_addr);
            }
            request->callback();
        }
        freeaddrinfo(result);
    }
}