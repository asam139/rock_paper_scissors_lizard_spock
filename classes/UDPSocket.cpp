//
// Created by Saul Moreno Abril on 30/06/2018.
//

#include "UDPSocket.h"

#include <iostream>
#include "SocketAddress.h"

#ifdef __APPLE__

UDPSocket::UDPSocket(int inSocket) : socket_(inSocket) {}

UDPSocket::~UDPSocket() {
    close(socket_);
}

#elif _WIN64

UDPSocket::UDPSocket(SOCKET inSocket) : socket_(inSocket) {}

UDPSocket::~UDPSocket() {
    closesocket(socket_);
}

#endif


#ifdef __APPLE__

#elif _WIN64

#endif

int UDPSocket::bindTo(const SocketAddress& inToAddress) {
    const int err = bind(socket_, inToAddress.getSockAddr(), inToAddress.getSize());
    if (err != 0) {
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

int UDPSocket::sendTo(const void* inData, int inLen, const SocketAddress& inTo) {
    const ssize_t byteSentCount = sendto(socket_, static_cast<const char*>(inData), inLen,
                                     0,
                                     inTo.getSockAddr(), inTo.getSize());
    if (byteSentCount >= 0) {
        std::cout << "Sent " << byteSentCount << " bytes" << std::endl;
        return byteSentCount;
    } else {
        std::cout << "Error Sending Data" << std::endl;
#ifdef __APPLE__
        return errno;
#elif _WIN64
        return WSAGetLastError();
#endif
    }
}
int UDPSocket::receiveFrom(void* inBuffer, size_t inLen, SocketAddress& outFrom) {
    socklen_t fromLength = outFrom.getSize();
    const ssize_t readByteCount = recvfrom(socket_, static_cast<char *>(inBuffer), inLen,
                                       0, outFrom.getSockAddr(), &fromLength);
    if (readByteCount >= 0) {
        std::cout << "Received " << readByteCount << " bytes" << std::endl;
        return readByteCount;
    } else {
        std::cout << "Error Binding Socket" << std::endl;
#ifdef __APPLE__
        return errno;
#elif _WIN64
        return WSAGetLastError();
#endif
    }
}


UDPSocketPtr UDPSocket::CreateUDPSocket(SocketAddressFamily inFamily) {
#ifdef __APPLE__
    const int s = socket(inFamily, SOCK_DGRAM, IPPROTO_UDP);
    if (s != 0) {
#elif _WIN64
    const SOCKET s = socket(inFamily, SOCK_DGRAM, IPPROTO_UDP);
    if (s != INVALID_SOCKET) {
#endif
        return UDPSocketPtr(new UDPSocket(s));
    } else {
        std::cout << "Error Creating UDP Socket" << std::endl;
        return nullptr;
    }
}