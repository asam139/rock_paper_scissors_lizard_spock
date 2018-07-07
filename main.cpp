#include <iostream>
#include <limits>
#include <cerrno>
#include <fcntl.h>      // for opening socket
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <netdb.h>
#include <unistd.h>     // for closing socket
#include <sys/socket.h>
#include <sys/types.h>
#include <vector>
#include <thread>

#include "classes/SocketAddress.h"
#include "classes/TCPSocket.h"

#define BUFFER_SIZE 1024

enum Mode : int8_t {
    ModeServer = 0,
    ModeClient = 1
};

void error(const char *msg)
{
    printf("Error: %s\n", msg);
    exit(EXIT_SUCCESS);
}

void message(const char *msg)
{
    printf("-> %s\n", msg);
}

class ClientHandler {
public:
    void operator()(TCPSocketPtr tcpSocketPtr){
        try {
            char buffer[BUFFER_SIZE];
            bool isFinished = false;

            auto welcomeMessage = "Start!";
            if (tcpSocketPtr->sendTo(welcomeMessage, strlen(welcomeMessage)) < 0) {
                error("Sending data");
            }

            do {
                auto welcomeMessage = "Rock";
                if (tcpSocketPtr->sendTo(welcomeMessage, strlen(welcomeMessage)) < 0) {
                    error("Sending data");
                }

                bzero(buffer, strlen(buffer));
                ssize_t bytes = tcpSocketPtr->receiveFrom(buffer, BUFFER_SIZE);
                if (bytes < 0) {
                    error("Receiving data");
                    isFinished = true;
                } else if (bytes == 0 ) {
                    message("Finished connection");
                    isFinished = true;
                }
                message(buffer);

            } while(!isFinished);
        } catch(...){}

        error("Connection with client was lost");
    }

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
        int portno;
        char buffer[BUFFER_SIZE];
        std::vector<std::unique_ptr<std::thread>> threads;

        std::cout << "\nInsert the port: ";
        std::cin >> portno;
        std::cout << "Port: " << portno << std::endl;

        SocketAddress *serverSocketAddress = new SocketAddress(INADDR_ANY, portno);
        std::unique_ptr<SocketAddress> serverSocketAddress_ptr (serverSocketAddress);

        TCPSocketPtr serverTCPSocketPtr = TCPSocket::CreateTCPSocket(INET);
        if (serverTCPSocketPtr == nullptr) {
            error("Creating socket");
        }

        if (serverTCPSocketPtr->bindTo(*serverSocketAddress_ptr) < 0) {
            error("On binding");
        }

        // This listenTo() call tells the socket to listen to the incoming connections.
        // The listenTo() function places all incoming connection into a backlog queue
        // until accept() call accepts the connection.
        // Here, we set the maximum size for the backlog queue to 5.
        serverTCPSocketPtr->listenTo(5);

        // The accept() call actually accepts an incoming connection
        SocketAddress *clientSocketAddress = new SocketAddress();
        std::unique_ptr<SocketAddress> clientSocketAddress_ptr (clientSocketAddress);

        // This accept() function will write the connecting client's address info
        // into the the address structure and the size of that structure is clilen.
        // The accept() returns a new socket file descriptor for the accepted connection.
        // So, the original socket file descriptor can continue to be used
        // for accepting new connections while the new socker file descriptor is used for
        // communicating with the connected client.
        bool isFinished = false;
        do {
            try {
                TCPSocketPtr clientTCPSocketPtr = serverTCPSocketPtr->acceptCon(*clientSocketAddress_ptr);
                if (clientTCPSocketPtr == nullptr) {
                    error("Accepting connection");
                }

                printf("Server: got connection from %s port %d\n",
                       inet_ntoa(clientSocketAddress_ptr->getAsSockAddrIn()->sin_addr),
                       ntohs(clientSocketAddress_ptr->getAsSockAddrIn()->sin_port));

                std::unique_ptr<std::thread> thread_ptr(new std::thread(ClientHandler(), clientTCPSocketPtr));
                threads.push_back(std::move(thread_ptr));
            } catch (...) {

            }

        } while (!isFinished);


    return EXIT_SUCCESS;
    }
    else
    {
        int portno;
        char hostname[BUFFER_SIZE];
        char buffer[BUFFER_SIZE];

        std::cout << "\nInsert the hostname: ";
        std::cin >> hostname;
        std::cout << "Hostname: " << hostname << std::endl;

        std::cout << "\nInsert the port: ";
        std::cin >> portno;
        std::cout << "Port: " << portno << std::endl;
        fflush(stdin);

        SocketAddress *serverSocketAddress = new SocketAddress(hostname, portno);
        std::unique_ptr<SocketAddress> serverSocketAddress_ptr (serverSocketAddress);

        TCPSocketPtr tcpSocketPtr = TCPSocket::CreateTCPSocket(INET);
        if (tcpSocketPtr == nullptr) {
            error("Creating socket");
        }

        if (tcpSocketPtr->connectTo(*serverSocketAddress_ptr) < 0) {
            error("Connecting");
        }

        bool isFinished = false;
        do {
            bzero(buffer, strlen(buffer));
            if (tcpSocketPtr->receiveFrom(buffer, BUFFER_SIZE) <= 0) {
                error("Reading from socket");
            }
            message(buffer);

            bzero(buffer, strlen(buffer));
            std::cout << "Please enter the message: ";
            //std::cin.clear();
            //std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            fgets(buffer, BUFFER_SIZE, stdin);

            if('\n' == buffer[strlen(buffer) - 1]) {
                buffer[strlen(buffer) - 1] = '\0';
            }

            size_t buffer_len = strlen(buffer);
            if (buffer_len > 0) {
                if (tcpSocketPtr->sendTo(buffer, buffer_len) <= 0) {
                    error("Writing to socket");
                }
            }
        } while (!isFinished);



        return 0;
    }
}