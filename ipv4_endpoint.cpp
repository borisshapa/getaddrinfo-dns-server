//
// Created by borisshapa on 24.12.2019.
//

#include <sstream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "ipv4_endpoint.h"

ipv4_endpoint::ipv4_endpoint(uint32_t addr, uint16_t port)
        : addr_net(htonl(addr))
        , port_net(htons(port)) {}

uint16_t ipv4_endpoint::port() const {
    return port_net;
}

uint32_t ipv4_endpoint::address() const {
    return addr_net;
}

std::string ipv4_endpoint::to_string() const {
    std::stringstream ss;
    ss << *this;
    return ss.str();
}

std::ostream& operator<<(std::ostream& os, ipv4_endpoint const& endpoint) {
    in_addr tmp{};
    tmp.s_addr = endpoint.address();
    os << inet_ntoa(tmp) << ':' << ntohs(endpoint.port());
    return os;
}