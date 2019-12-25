cmake_minimum_required(VERSION 3.15)
project(server)

set(CMAKE_CXX_STANDARD 17)

add_executable(server main.cpp socket.cpp socket.h epoll_wrapper.cpp epoll_wrapper.h raii_fd.cpp raii_fd.h echo_server.cpp echo_server.h ipv4_endpoint.cpp ipv4_endpoint.h)