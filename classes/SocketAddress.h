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

#include "SocketAddressFamily.h"

class SocketAddress;

typedef std::shared_ptr<SocketAddress> SocketAddressPtr;

class SocketAddress {
public:
    SocketAddress();
    SocketAddress(char* hostname, uint16_t port);
    SocketAddress(uint32_t address, uint16_t port);
    SocketAddress(const sockaddr& sockAddr);

    static SocketAddressPtr CreateIPv4FromString(const std::string& in);

    socklen_t getSize() const;
    sockaddr* const getSockAddr() const;
    sockaddr_in* const getAsSockAddrIn() const;
private:
    sockaddr sockAddr_;
};



#endif //ROCK_PAPER_SCISSORS_LIZARD_SPOCK_SOCKETADDRESS_H
