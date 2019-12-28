//
// Created by borisshapa on 27.12.2019.
//

#ifndef SERVER_DNS_THREAD_POOL_H
#define SERVER_DNS_THREAD_POOL_H


#include <cstdint>
#include <string>
#include <functional>
#include <sys/socket.h>
#include <netinet/in.h>
#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>
#include <atomic>

struct dns_thread_pool {

    typedef std::function<void()> callback_t;

    struct request;

    explicit dns_thread_pool(size_t threads_count);

    ~dns_thread_pool();

    void resolve(std::string const &hostname, size_t id, callback_t callback);

    std::string get_response(size_t id);

    struct request {
        request(std::string hostname, size_t id, callback_t callback);

        std::string hostname;
        size_t id;
        callback_t callback;
    };

private:
    std::atomic_bool finished;
    std::vector<std::thread> resolvers;

    std::mutex m_req;
    std::mutex m_resp;
    std::condition_variable cv;
    std::queue<request *> req_q;
    std::unordered_map<size_t, std::vector<struct in_addr>> resp;
    std::unordered_map<size_t, std::atomic_uint32_t> req_counts;
    void resolver();
};


#endif //SERVER_DNS_THREAD_POOL_H
