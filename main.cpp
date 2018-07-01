#include <iostream>
#include <errno.h>
#include <fcntl.h>      // for opening socket
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>     // for closing socket
#include <sys/socket.h>
#include <sys/types.h>

#include "classes/UDPSocket.h"
#include "classes/SocketAddress.h"

enum Mode : int8_t {
    ModeServer = 0,
    ModeClient = 1
};

int main(int argc , char *argv[]) {
    Mode mode = ModeServer;

    std::cout << "\n-------------------------------------" << std::endl;
    std::cout << "\t\t\tWelcome to" << std::endl;
    std::cout << "-------------------------------------" << std::endl;
    std::cout << "Rock Paper Scissors Lizard Spock - Game" << std::endl;
    std::cout << "-------------------------------------" << std::endl;

    std::cout << "Instructions" << std::endl;
    std::cout << "Do you want to init like server or client?" << std::endl;
    std::cout << "\t0 - Server" << std::endl;
    std::cout << "\t1 - Client" << std::endl;
    int input;
    std::cin >> input;

    if (input > 1) {
        std::cout << "The mode is not available" << std::endl;
        getchar();
        return EXIT_FAILURE;
    }
    mode = static_cast<Mode>(input);

    if (mode == ModeServer)
    {
        UDPSocketPtr udpSocketPtr = UDPSocket::CreateUDPSocket(INET);

        SocketAddressPtr addressPtr = SocketAddress::CreateIPv4FromString("127.0.0.1:7000");
        udpSocketPtr->bindTo(*addressPtr.get());
    }
    else
    {

    }

    {



    }

    getchar();
    return 0;
}