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

#include "effolkronium/random.hpp"

// get base random alias which is auto seeded and has static API and internal state
using Random = effolkronium::random_static;

enum Mode : int8_t {
    ModeServer = 0,
    ModeClient = 1
};

enum GameState : int8_t {
    NotStarted = 0,
    Started = 1,
    Ended = 2
};

enum GameMessageType : int8_t {
    Informative = 0,
    Element = 1
};

enum GameElement : int8_t {
    Rock = 0,
    Paper = 1,
    Scissors = 2,
    Lizard = 3,
    Spock = 4
};

char* stringFromGameElement(GameElement element) {
    switch (element) {
        case Rock:
            return "Rock";
        case Paper:
            return "Paper";
        case Scissors:
            return "Scissors";
        case Lizard:
            return "Lizard";
        case Spock:
            return "Spock";
        default:
            return "";
    }
}

struct GameMessage {
    GameState gameState;
    GameMessageType messageType;
    char text[256];
    GameElement element;
};
#define BUFFER_SIZE sizeof(GameMessage)


void error(const char *msg)
{
    printf("Error: %s\n", msg);
}

void message(const char *msg)
{
    printf("-> %s\n", msg);
}

class ClientHandler {
private:

public:
    void operator()(TCPSocketPtr tcpSocketPtr){
        try {
            char buffer[BUFFER_SIZE];
            bool isFinished = false;
            Random localRandom{};

            struct GameMessage gameMessage;
            gameMessage.gameState = Started;
            gameMessage.messageType = Informative;
            strcpy(gameMessage.text, "Start!");

            bzero(buffer, strlen(buffer));
            memcpy(buffer, &gameMessage, BUFFER_SIZE);
            if (tcpSocketPtr->sendTo(buffer, BUFFER_SIZE) < 0) {
                message("Sending data");
            }

            do {
                bzero(buffer, strlen(buffer));
                ssize_t bytes = tcpSocketPtr->receiveFrom(buffer, BUFFER_SIZE);
                if (bytes < 0) {
                    message("Receiving data");
                    isFinished = true;
                    continue;
                } else if (bytes == 0 ) {
                    message("Finished connection");
                    isFinished = true;
                    continue;
                }

                memcpy(&gameMessage, buffer, BUFFER_SIZE);
                if (gameMessage.gameState == Started) {
                    if (gameMessage.messageType == Element) {
                        std::cout << "Element: ";
                        std::cout << stringFromGameElement(gameMessage.element) << std::endl;
                    }
                } else if (gameMessage.gameState == Ended) {
                    isFinished = true;
                    continue;
                }

                // New Element
                GameElement element = (GameElement)localRandom.get((int)(Rock), (int)Spock);


                // New game message
                gameMessage.gameState = Started;
                gameMessage.messageType = Informative;
                strcpy(gameMessage.text, "Round 1: Lost");
                gameMessage.element = element;

                bzero(buffer, strlen(buffer));
                memcpy(buffer, &gameMessage, BUFFER_SIZE);
                if (tcpSocketPtr->sendTo(buffer, BUFFER_SIZE) < 0) {
                    message("Sending data");
                    isFinished = true;
                    continue;
                }


            } while(!isFinished);
        } catch(...){}

        message("Connection with client was lost");
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
        std::vector<std::unique_ptr<std::thread>> threads;

        std::cout << "\nInsert the port: ";
        std::cin >> portno;
        std::cout << "Port: " << portno << std::endl;

        SocketAddress *serverSocketAddress = new SocketAddress(INADDR_ANY, portno);
        std::unique_ptr<SocketAddress> serverSocketAddress_ptr (serverSocketAddress);

        TCPSocketPtr serverTCPSocketPtr = TCPSocket::CreateTCPSocket(INET);
        if (serverTCPSocketPtr == nullptr) {
            error("Creating socket");
            exit(EXIT_SUCCESS);
        }

        if (serverTCPSocketPtr->bindTo(*serverSocketAddress_ptr) < 0) {
            error("On binding");
            exit(EXIT_SUCCESS);
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
    }
    else
    {
        int portno;
        char hostname[512];
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
            exit(EXIT_SUCCESS);
        }

        if (tcpSocketPtr->connectTo(*serverSocketAddress_ptr) < 0) {
            error("Connecting");
            exit(EXIT_SUCCESS);
        }


        // Wait message to start game
        bool isStarted = false;

        bzero(buffer, strlen(buffer));
        if (tcpSocketPtr->receiveFrom(buffer, BUFFER_SIZE) <= 0) {
            error("Reading from socket");
            exit(EXIT_SUCCESS);
        }

        struct GameMessage gameMessage;
        memcpy(&gameMessage, buffer, BUFFER_SIZE);
        if (gameMessage.gameState == Started) {
            if (gameMessage.messageType == Informative) {
                message(gameMessage.text);
            }
            isStarted = true;
        }

        do {
            //std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            /*fgets(buffer, BUFFER_SIZE, stdin);
            if('\n' == buffer[strlen(buffer) - 1]) {
                buffer[strlen(buffer) - 1] = '\0';
            }*/

            int option;
            std::cout << "\nInstructions: " << std::endl;
            std::cout << "\t- Rock: 0" << std::endl;
            std::cout << "\t- Paper: 1" << std::endl;
            std::cout << "\t- Scissors: 2" << std::endl;
            std::cout << "\t- Lizard: 3" << std::endl;
            std::cout << "\t- Spock: 4" << std::endl;
            std::cout << "Your turn: ";
            std::cin >> option;
            GameElement element = (GameElement)option;
            if (element < Rock || element > Spock) {
                error("Wrong hand");
                isStarted = false;
                continue;
            }

            gameMessage.gameState = Started;
            gameMessage.messageType = Element;
            gameMessage.element = element;
            strcpy(gameMessage.text, "");

            bzero(buffer, strlen(buffer));
            memcpy(buffer, &gameMessage, BUFFER_SIZE);
            if (tcpSocketPtr->sendTo(buffer, BUFFER_SIZE) < 0) {
                error("Sending data");
                isStarted = false;
                continue;
            }

            // Receive new game message
            bzero(buffer, strlen(buffer));
            if (tcpSocketPtr->receiveFrom(buffer, BUFFER_SIZE) <= 0) {
                error("Reading from socket");
                isStarted = false;
                continue;
            }
            memcpy(&gameMessage, buffer, BUFFER_SIZE);

            // Process new game message
            if (gameMessage.gameState == Started) {
                if (gameMessage.messageType == Informative) {
                    std::cout << "\nEnemy Element: ";
                    std::cout << stringFromGameElement(gameMessage.element) << std::endl;
                    message(gameMessage.text);
                }

            } else if (gameMessage.gameState == Ended){
                if (gameMessage.messageType == Informative) {
                    message(gameMessage.text);
                }
                isStarted = false;
            }


        } while (isStarted);
    }

    return EXIT_SUCCESS;
}