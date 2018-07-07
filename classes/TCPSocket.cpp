//
// Created by Saul Moreno Abril on 04/07/2018.
//

#include "TCPSocket.h"
#include "SocketAddress.h"

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
#ifdef __APPLE__
        return -errno;
    }
    return 0;
#elif _WIN64
        return -WSAGetLastError();
    }
    return NO_ERROR;
#endif
}
int TCPSocket::bindTo(const SocketAddress& inBindAddress) const {
    const int error = bind(socket_, inBindAddress.getSockAddr(), inBindAddress.getSize());
    if (error != 0) {
#ifdef __APPLE__
        return -errno;
    }
    return 0;
#elif _WIN64
        return -WSAGetLastError();
    }
    return NO_ERROR;
#endif
}
int TCPSocket::listenTo(int inBackLog) const {
    const int err = listen(socket_, inBackLog);
    if (err < 0) {
#ifdef __APPLE__
        return -errno;
    }
    return 0;
#elif _WIN64
        return -WSAGetLastError();
    }
    return NO_ERROR;
#endif
}

ssize_t TCPSocket::sendTo(const void* inData, size_t inLen) const {
    ssize_t bytesSentCount = send(socket_, inData, inLen, 0);
    if (bytesSentCount < 0) {
#ifdef __APPLE__
        return -errno;
    }
#elif _WIN64
        return -WSAGetLastError();
    }
#endif
    return bytesSentCount;
}
ssize_t TCPSocket::receiveFrom(void* inData, size_t inLen) const {
    ssize_t bytesReceivedCount = recv(socket_, inData, inLen, 0);
    if (bytesReceivedCount < 0) {
#ifdef __APPLE__
        return -errno;
    }
#elif _WIN64
        return -WSAGetLastError();
    }
#endif
    return bytesReceivedCount;
}

TCPSocketPtr TCPSocket::acceptCon(SocketAddress& inFromAddress) const {
    socklen_t length = inFromAddress.getSize();
#ifdef __APPLE__
    const int newSocket = accept(socket_, inFromAddress.getSockAddr(), &length);
    if (newSocket >= 0) {
#elif _WIN64
        const SOCKET newSocket = accept(socket_, inFromAddress.getSockAddr(), &length);
    if (newSocket != INVALID_SOCKET) {
#endif
        return TCPSocketPtr(new TCPSocket(newSocket));
    } else {
        return nullptr;
    }
}


TCPSocketPtr TCPSocket::CreateTCPSocket(SocketAddressFamily inFamily) {
#ifdef __APPLE__
    const int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s != 0) {
#elif _WIN64
        const SOCKET s = socket(inFamily, SOCK_STREAM, IPPROTO_TCP);
    if (s != INVALID_SOCKET) {
#endif
        return TCPSocketPtr(new TCPSocket(s));
    } else {
        return nullptr;
    }
}