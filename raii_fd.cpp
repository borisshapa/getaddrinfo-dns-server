//
// Created by borisshapa on 23.12.2019.
//

#include <unistd.h>
#include <cerrno>
#include <iostream>
#include <sys/signalfd.h>
#include <csignal>
#include "raii_fd.h"

raii_fd::raii_fd() : fd(-1) {}

raii_fd::raii_fd(int fd) : fd(fd) {}

raii_fd::~raii_fd() {
    if (fd != -1) {
        int res = close(fd);
        if (res == -1) {
            std::cerr << "Can't close file descriptor " + std::to_string(fd) << std::endl;
        }
        fd = -1;
    }
}

int raii_fd::get_fd() const {
    return fd;
}

raii_fd raii_fd::signal_fd(const std::vector<uint8_t>& signals) {
    sigset_t mask;
    sigemptyset(&mask);
    for (auto const& sig : signals) {
        sigaddset(&mask, sig);
    }

    int proc_mask = sigprocmask(SIG_BLOCK, &mask, nullptr);
    if (proc_mask == -1) {
        std::cerr << "Can't create sigprocmask during creating signal_fd." << std::endl;
    }
    raii_fd res = signalfd(-1, &mask, 0);
    if (!res.valid_existing()) {
        std::cerr << "Can't create signalfd." << std::endl;
    }
    return res;
}

bool raii_fd::valid_existing() const{
    return fd != -1;
}

raii_fd::raii_fd(raii_fd &&other) noexcept {
    swap(*this, other);
    other.fd = -1;
}

raii_fd &raii_fd::operator=(raii_fd &&rhs) noexcept {
    swap(*this, rhs);
    rhs.fd = -1;
    return *this;
}

void swap(raii_fd &a, raii_fd &b) {
    std::swap(a.fd, b.fd);
}