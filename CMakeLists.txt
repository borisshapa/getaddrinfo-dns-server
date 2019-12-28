cmake_minimum_required(VERSION 3.15)
project(server)

set(CMAKE_CXX_STANDARD 17)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_executable(server
        main.cpp
        socket.cpp socket.h
        epoll_wrapper.cpp epoll_wrapper.h
        raii_fd.cpp raii_fd.h
        echo_server.cpp echo_server.h
        ipv4_endpoint.cpp ipv4_endpoint.h
        dns_thread_pool.cpp dns_thread_pool.h
        dns_server.cpp dns_server.h timer.cpp timer.h)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=leak -g")