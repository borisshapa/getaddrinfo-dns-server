//
// Created by borisshapa on 23.12.2019.
//

#ifndef SERVER_RAII_FD_H
#define SERVER_RAII_FD_H


#include <sys/types.h>
#include <vector>

struct raii_fd {
    raii_fd();

    raii_fd(int fd);

    raii_fd(raii_fd const &other) = delete;

    raii_fd(raii_fd &&other);

    raii_fd &operator=(raii_fd &&rhs);

    ~raii_fd();

    int get_fd() const;

    ssize_t read(void *buf, size_t count);

    ssize_t write(void const *buf, size_t count);

    bool valid_existing() const;

    static raii_fd signal_fd(const std::vector<uint8_t> &signals);

    friend void swap(raii_fd &a, raii_fd &b);

private:
    int fd;
};

void swap(raii_fd &a, raii_fd &b);

#endif //SERVER_RAII_FD_H
