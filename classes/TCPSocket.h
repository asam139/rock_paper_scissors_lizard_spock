//
// Created by Saul Moreno Abril on 04/07/2018.
//

#ifndef ROCK_PAPER_SCISSORS_LIZARD_SPOCK_TCPSOCKET_H
#define ROCK_PAPER_SCISSORS_LIZARD_SPOCK_TCPSOCKET_H

#include <cstdio>
#include <string>
#include <memory>

#ifdef __APPLE__
#include <errno.h>
#include <fcntl.h>      // for opening socket

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
class TCPSocket;

typedef std::shared_ptr<TCPSocket> TCPSocketPtr;

class TCPSocket {
public:
    ~TCPSocket();
    int connectTo(const SocketAddress& inAddress) const;
    int bindTo(const SocketAddress& inToAddress) const;
    int listenTo(int inBackLog = 32) const;
    int sendTo(const void* inData, int inLen) const;
    int receiveFrom(void* inBuffer, int inLen) const;

    std::shared_ptr< TCPSocket > acceptCon(SocketAddress& inFromAddress) const;
private:
    friend class SocketUtil;

#ifdef __APPLE__
    TCPSocket(int inSocket);
    int socket_;

#elif _WIN64
    TCPSocket(SOCKET inSocket) : socket_(inSocket) {}
    SOCKET socket_;

#endif
};


#endif //ROCK_PAPER_SCISSORS_LIZARD_SPOCK_TCPSOCKET_H
