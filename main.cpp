#include <iostream>
#include <limits>
#include <errno.h>
#include <fcntl.h>      // for opening socket
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>     // for closing socket
#include <sys/socket.h>
#include <sys/types.h>

#include "classes/SocketAddress.h"
#include "classes/TCPSocket.h"

#define BUFFER_SIZE 1024

enum Mode : int8_t {
    ModeServer = 0,
    ModeClient = 1
};

void error(const char *msg)
{
    perror(msg);
    exit(EXIT_SUCCESS);
}

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
        int sockfd, newsockfd, portno;
        socklen_t clilen;
        char buffer[BUFFER_SIZE];
        struct sockaddr_in serv_addr, cli_addr;
        int n;

        std::cout << "\nInsert the port:" << std::endl;
        std::cin >> portno;
        std::cout << "Port:" << portno << std::endl;

        // create a socket
        // socket(int domain, int type, int protocol)
        sockfd =  socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
            error("ERROR opening socket");

        // clear address structure
        bzero((char *) &serv_addr, sizeof(serv_addr));

        /* setup the host_addr structure for use in bind call */
        // server byte order
        serv_addr.sin_family = AF_INET;

        // automatically be filled with current host's IP address
        serv_addr.sin_addr.s_addr = INADDR_ANY;

        // convert short integer value for port must be converted into network byte order
        serv_addr.sin_port = htons(portno);

        // bind(int fd, struct sockaddr *local_addr, socklen_t addr_length)
        // bind() passes file descriptor, the address structure,
        // and the length of the address structure
        // This bind() call will bind  the socket to the current IP address on port, portno
        if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
            error("ERROR on binding");

        // This listen() call tells the socket to listen to the incoming connections.
        // The listen() function places all incoming connection into a backlog queue
        // until accept() call accepts the connection.
        // Here, we set the maximum size for the backlog queue to 5.
        listen(sockfd, 5);

        // The accept() call actually accepts an incoming connection
        clilen = sizeof(cli_addr);

        // This accept() function will write the connecting client's address info
        // into the the address structure and the size of that structure is clilen.
        // The accept() returns a new socket file descriptor for the accepted connection.
        // So, the original socket file descriptor can continue to be used
        // for accepting new connections while the new socker file descriptor is used for
        // communicating with the connected client.
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0)
            error("ERROR on accept");

        printf("Server: got connection from %s port %d\n",
               inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));


        // This send() function sends the 13 bytes of the string to the new socket
        send(newsockfd, "Hello, world!\n", 13, 0);

        bzero(buffer,256);

        n = read(newsockfd,buffer,255);
        if (n < 0) error("ERROR reading from socket");
        printf("Here is the message: %s\n",buffer);

        close(newsockfd);
        close(sockfd);
        return EXIT_SUCCESS;
    }
    else
    {
        int n;
        u_int16_t portno;
        char hostname[BUFFER_SIZE];
        char buffer[BUFFER_SIZE];

        std::cout << "\nInsert the hostname:" << std::endl;
        std::cin >> hostname;
        std::cout << "Hostname:" << hostname << std::endl;

        std::cout << "\nInsert the port:" << std::endl;
        std::cin >> portno;
        std::cout << "Port:" << portno << std::endl;
        fflush(stdin);

        SocketAddress *serverSocketAddress = new SocketAddress(hostname, portno);
        std::unique_ptr<SocketAddress> serverSocketAddress_ptr (serverSocketAddress);

        TCPSocketPtr tcpSocketPtr = TCPSocket::CreateTCPSocket(INET);
        if (tcpSocketPtr == nullptr) {
            error("ERROR creating socket");
        }

        if (tcpSocketPtr->connectTo(*serverSocketAddress_ptr) < 0) {
            error("ERROR connecting");
        }

        printf("Please enter the message: ");
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        fgets(buffer,BUFFER_SIZE, stdin);

        n = tcpSocketPtr->sendTo(buffer, strlen(buffer));
        if (n < 0) {
            error("ERROR writing to socket");
        }

        bzero(buffer,256);
        n = tcpSocketPtr->receiveFrom(buffer, strlen(buffer));
        if (n < 0) {
            error("ERROR reading from socket");
        }
        printf("%s\n", buffer);

        return 0;
    }


    getchar();
    return 0;
}