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
        std::cerr << "CLOSE " << fd << std::endl;
        int res = close(fd);
        std::cerr << res << std::endl;
        if (res == -1 && errno != EAGAIN) {
            std::cerr << "Can't close file descriptor " + std::to_string(fd) << std::endl;
        }
        fd = -1;
    }
}

int raii_fd::get_fd() const {
    return fd;
}

ssize_t raii_fd::read(void *buf, size_t count) {
    ssize_t res = ::read(fd, buf, count);
    if (res == -1) {
        // TODO: Check what is it
        if (errno != EWOULDBLOCK && errno != EAGAIN) {
            std::cerr << "Can't read from file descriptor " + std::to_string(fd) << std::endl;
        }
    }
    return res;
}

ssize_t raii_fd::write(void const *buf, size_t count) {
    ssize_t res = ::write(fd, buf, count);
    if (res == -1) {
        // TODO: Check what is it
        if (errno != EWOULDBLOCK && errno != EAGAIN) {
            std::cerr << "Can't write from file descriptor " << std::to_string(fd) << std::endl;
        }
    }
    return res;
}

raii_fd raii_fd::signal_fd(const std::vector<uint8_t>& signals) {
    sigset_t mask;
    sigemptyset(&mask);
    for (auto const& sig : signals) {
        sigaddset(&mask, sig);
    }
    // TODO: Check what is it
    if (sigprocmask(SIG_BLOCK, &mask, nullptr) == -1) {
        std::cerr << "Can't create sigprocmask during creating signal_fd." << std::endl;
    }
    raii_fd res = signalfd(-1, &mask, SFD_CLOEXEC | SFD_NONBLOCK);
    if (!res.valid_existing()) {
        std::cerr << "Can't create signalfd." << std::endl;
    }
    return res;
}

bool raii_fd::valid_existing() const{
    return fd != -1;
}

raii_fd::raii_fd(raii_fd &&other) {
    swap(*this, other);
}

raii_fd &raii_fd::operator=(raii_fd &&rhs) {
    swap(*this, rhs);
    return *this;
}

void swap(raii_fd &a, raii_fd &b) {
    std::swap(a.fd, b.fd);
}