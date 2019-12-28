#include "socket.h"
#include "epoll_wrapper.h"
#include "echo_server.h"
#include "dns_server.h"

int main() {
    epoll_wrapper epoll;
    dns_server server(epoll, ipv4_endpoint(INADDR_ANY, 45123));
    epoll.run();
}