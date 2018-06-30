//
// Created by Saul Moreno Abril on 30/06/2018.
//

#ifndef ROCK_PAPER_SCISSORS_LIZARD_SPOCK_SOCKETADDRESS_H
#define ROCK_PAPER_SCISSORS_LIZARD_SPOCK_SOCKETADDRESS_H

#include <stdio.h>
#include <string>
#include <memory>


#ifdef __APPLE__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#elif _WIN64

#endif

class SocketAddress;

typedef std::shared_ptr<SocketAddress> SocketAddressPtr;

class SocketAddress {
public:
    SocketAddress(uint32_t address, uint16_t port) {
        getAsSockAddrIn()->sin_family = AF_INET;
        getAsSockAddrIn()->sin_addr.s_addr = htonl(address);
        getAsSockAddrIn()->sin_port = htons(port);
    }
    SocketAddress(const sockaddr& sockAddr) {
        memcpy(&sockAddr_, &sockAddr, sizeof(sockaddr));
    }
    size_t getSize() const { return sizeof(sockaddr); }

    static SocketAddressPtr CreateIPv4FromString(const std::string& in);
private:
    sockaddr sockAddr_;
    sockaddr_in* getAsSockAddrIn() {
        return reinterpret_cast<sockaddr_in*>(&sockAddr_);
    }
};


#endif //ROCK_PAPER_SCISSORS_LIZARD_SPOCK_SOCKETADDRESS_H
