#include "socket.h"
#include "epoll_wrapper.h"
#include "echo_server.h"

int main() {
    epoll_wrapper epoll;
    echo_server server(epoll, ipv4_endpoint(INADDR_ANY, 45123));
    epoll.run();
}