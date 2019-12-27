//
// Created by borisshapa on 24.12.2019.
//

#ifndef SERVER_IPV4_ENDPOINT_H
#define SERVER_IPV4_ENDPOINT_H


#include <cstdint>
#include <string>

struct ipv4_endpoint {
    ipv4_endpoint(uint32_t addr, uint16_t port);

    uint16_t port() const;

    uint32_t address() const;

    friend std::ostream &operator<<(std::ostream& os, ipv4_endpoint const& endpoint);

private:
    uint16_t port_net;
    uint32_t addr_net;
};

std::ostream &operator<<(std::ostream &os, ipv4_endpoint const &endpoint);

#endif //SERVER_IPV4_ENDPOINT_H
