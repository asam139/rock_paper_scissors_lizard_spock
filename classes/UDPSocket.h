//
// Created by Saul Moreno Abril on 30/06/2018.
//

#ifndef ROCK_PAPER_SCISSORS_LIZARD_SPOCK_UDPSOCKET_H
#define ROCK_PAPER_SCISSORS_LIZARD_SPOCK_UDPSOCKET_H

#include <stdio.h>
#include <string>
#include <memory>

#ifdef __APPLE__
#include <errno.h>
#include <fcntl.h>      // for opening socket
#include <cstdio>
#include <cstring>
#include <netdb.h>
#include <unistd.h>     // for closing socket
#include <sys/socket.h>
#include <sys/types.h>

#elif _WIN64
#include <WinSock2.h>
#include <Ws2tcpip.h>

#endif

#include "SocketAddressFamily.h"

class SocketAddress;
class UDPSocket;

typedef std::shared_ptr<UDPSocket> UDPSocketPtr;

class UDPSocket {
public:
    ~UDPSocket();
    int bindTo(const SocketAddress& inToAddress);
    int sendTo(const void* inData, int inLen, const SocketAddress& inTo);
    int receiveFrom(void* inBuffer, size_t inLen, SocketAddress& outFrom);

    static UDPSocketPtr CreateUDPSocket(SocketAddressFamily inFamily);
private:

#ifdef __APPLE__
    UDPSocket(int inSocket);
    int socket_;

#elif _WIN64
    UDPSocket(SOCKET inSocket);
    SOCKET socket_;

#endif
};
typedef std::shared_ptr<UDPSocket> UDPSocketPtr;

#endif //ROCK_PAPER_SCISSORS_LIZARD_SPOCK_UDPSOCKET_H
