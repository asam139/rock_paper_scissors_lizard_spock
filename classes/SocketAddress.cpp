//
// Created by Saul Moreno Abril on 30/06/2018.
//

#include "SocketAddress.h"



SocketAddress::SocketAddress(char* hostname, uint16_t port) {
    bzero((char *) &sockAddr_, sizeof(sockAddr_));

    struct hostent* server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");

    }

    sockaddr_in* sockAddrIn = getAsSockAddrIn();
    sockAddrIn->sin_family = INET;
    bcopy(server->h_addr, (char *)&sockAddrIn->sin_addr.s_addr, server->h_length);
    sockAddrIn->sin_port = htons(port);
}

SocketAddress::SocketAddress(uint32_t address, uint16_t port) {
    sockaddr_in* sockAddrIn = getAsSockAddrIn();

    sockAddrIn->sin_family = INET;
    sockAddrIn->sin_addr.s_addr = htonl(address);
    sockAddrIn->sin_port = htons(port);
}

SocketAddress::SocketAddress(const sockaddr& sockAddr) {
    memcpy(&sockAddr_, &sockAddr, sizeof(sockaddr));
}


SocketAddressPtr SocketAddress::CreateIPv4FromString(const std::string& in) {
    const auto pos = in.find_last_of(':');
    std::string host, service;
    if (pos != std::string::npos) {
        host = in.substr(0, pos);
        service = in.substr(pos + 1);
    } else {
        host = in;
        service = "0"; //use default port...
    }
    addrinfo hint;
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = INET;
    addrinfo *list;
    const int error = getaddrinfo(host.c_str(), service.c_str(), &hint, &list);
    addrinfo *list_head = list;
    if (error != 0 && list != nullptr) {
        freeaddrinfo(list_head);
        return nullptr;
    }
    while (!list->ai_addr && list->ai_next) {
        list = list->ai_next;
    }
    if (!list->ai_addr) {
        freeaddrinfo(list_head);
        return nullptr;
    }
    auto result = std::make_shared<SocketAddress>(*list->ai_addr);
    freeaddrinfo(list_head);
    return result;
}

sockaddr* const SocketAddress::getSockAddr() const {
    return const_cast<sockaddr*>(&sockAddr_);
}

sockaddr_in* const SocketAddress::getAsSockAddrIn() const {
    return reinterpret_cast<sockaddr_in*>(const_cast<sockaddr*>(&sockAddr_));
}