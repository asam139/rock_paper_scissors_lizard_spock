//
// Created by Saul Moreno Abril on 04/07/2018.
//

#include "TCPSocket.h"
#include "SocketAddress.h"

#include <iostream>

#ifdef __APPLE__

TCPSocket::TCPSocket(int inSocket) : socket_(inSocket) {}

TCPSocket::~TCPSocket() {
    close(socket_);
}

#elif _WIN64

TCPSocket::TCPSocket(SOCKET inSocket) : socket_(inSocket) {}

TCPSocket::~TCPSocket() {
    closesocket(socket_);
}

#endif



int TCPSocket::connectTo(const SocketAddress& inAddress) const {
    const int err = connect(socket_, inAddress.getSockAddr(), inAddress.getSize());
    if (err < 0) {
        std::cout << "Error Connecting Socket" << std::endl;
#ifdef __APPLE__
        return errno;
    }
    return 0;
#elif _WIN64
        return WSAGetLastError();
    }
    return NO_ERROR;
#endif
}
int TCPSocket::bindTo(const SocketAddress& inBindAddress) const {
    const int error = bind(socket_, inBindAddress.getSockAddr(), inBindAddress.getSize());
    if (error != 0) {
        std::cout << "Error Binding Socket" << std::endl;
#ifdef __APPLE__
        return errno;
    }
    return 0;
#elif _WIN64
        return WSAGetLastError();
    }
    return NO_ERROR;
#endif
}
int TCPSocket::listenTo(int inBackLog) const {
    const int err = listen(socket_, inBackLog);
    if (err < 0) {
        std::cout << "Error Listening Socket" << std::endl;
#ifdef __APPLE__
        return errno;
    }
    return 0;
#elif _WIN64
        return -WSAGetLastError();
    }
    return NO_ERROR;
#endif
}

int TCPSocket::sendTo(const void* inData, int inLen) const {
    int bytesSentCount = send(socket_, static_cast<const char*>(inData), inLen, 0);
    if (bytesSentCount < 0) {
        std::cout << "Error Sending Data" << std::endl;
#ifdef __APPLE__
        return errno;
    }
    return bytesSentCount;
#elif _WIN64
        return -WSAGetLastError();
    }
    return bytesSentCount;
#endif
}
int TCPSocket::receiveFrom(void* inData, int inLen) const {
    int bytesReceivedCount = recv(socket_, static_cast<char *>(inData), inLen, 0);
    if (bytesReceivedCount < 0) {
        std::cout << "Error Receiving Data" << std::endl;
#ifdef __APPLE__
        return errno;
    }
    return bytesReceivedCount;
#elif _WIN64
        return -WSAGetLastError();
    }
    return bytesReceivedCount;
#endif
}

TCPSocketPtr TCPSocket::acceptCon(SocketAddress& inFromAddress) const {
    socklen_t length = inFromAddress.getSize();
#ifdef __APPLE__
    const int newSocket = accept(socket_, inFromAddress.getSockAddr(), &length);
    if (newSocket != 0) {
#elif _WIN64
        const SOCKET newSocket = accept(socket_, inFromAddress.getSockAddr(), &length);
    if (newSocket != INVALID_SOCKET) {
#endif
        return TCPSocketPtr(new TCPSocket(newSocket));
    } else {
        std::cout << "Error Accepting Socket" << std::endl;
        return nullptr;
    }
}
