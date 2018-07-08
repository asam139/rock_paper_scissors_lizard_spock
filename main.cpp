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
    Started = 0,
    Ended = 1
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
    auto text = (char *)"";
    switch (element) {
        case Rock:
            text = (char *)"Rock";
            break;
        case Paper:
            text = (char *)"Paper";
            break;
        case Scissors:
            text = (char *)"Scissors";
            break;
        case Lizard:
            text = (char *)"Lizard";
            break;
        case Spock:
            text = (char *)"Spock";
            break;
        default:
            break;
    }

    return text;
}

enum GameRound : int8_t {
    Lost = -1,
    Tie = 0,
    Win = 1,
};

// if the first wins it returns Win (GameRound enum)
// it it is a tie it returns Tie (GameRound enum)
// and if the seconds wins it returns Lost (GameRound enum)
GameRound whoWin(GameElement first, GameElement second) {
    if (first == second) {
        return Tie;
    }

    switch (first) {
        case Rock:
            return second == Scissors || second == Lizard ? Win : Lost;
        case Paper:
            return second == Rock || second == Spock ? Win : Lost;
        case Scissors:
            return second == Paper || second == Lizard ? Win : Lost;
        case Lizard:
            return second == Paper || second == Spock ? Win : Lost;
        case Spock:
            return second == Scissors || second == Rock ? Win : Lost;
        default:
            return Tie;
    }
}

struct GameMessage {
    GameState gameState;
    GameMessageType messageType;
    char text[256];
    GameElement element;
};
#define BUFFER_SIZE sizeof(GameMessage)

class ClientHandler {
private:

public:
    void operator()(const TCPSocketPtr &tcpSocketPtr){
        try {
            char buffer[BUFFER_SIZE];
            bool isFinished = false;
            Random localRandom{};

            struct GameMessage gameMessage = {};
            gameMessage.gameState = Started;
            gameMessage.messageType = Informative;
            strcpy(gameMessage.text, "Start!");

            bzero(buffer, strlen(buffer));
            memcpy(buffer, &gameMessage, BUFFER_SIZE);
            if (tcpSocketPtr->sendTo(buffer, BUFFER_SIZE) < 0) {
                std::cout << "Error: sending data" << std::endl;
                isFinished = true;
            }

            const int maxRounds = 5;
            int round = 0;
            int roundWonByClient = 0;
            int roundWonByServer = 0;
            const int maxRoundToWin = maxRounds / 2 + 1;
            do {
                round += 1;

                bzero(buffer, strlen(buffer));
                ssize_t bytes = tcpSocketPtr->receiveFrom(buffer, BUFFER_SIZE);
                if (bytes < 0) {
                    std::cout << "Error: receiving data" << std::endl;
                    isFinished = true;
                    continue;
                } else if (bytes == 0 ) {
                    std::cout << "Finished connection" << std::endl;
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
                auto clientElement = gameMessage.element;
                auto serverElement = (GameElement)localRandom.get((int)(Rock), (int)Spock);

                GameRound wWin = whoWin(clientElement, serverElement);
                if (wWin == Win) {
                    roundWonByClient += 1;
                } else if (wWin == Lost) {
                    roundWonByServer += 1;
                }


                if (roundWonByClient >= maxRoundToWin ||
                    roundWonByServer >= maxRoundToWin ||
                    round >= maxRounds) {
                    char *text;
                    if (roundWonByClient > roundWonByServer) {
                        text = (char*)"YOU WIN!";
                    } else if (roundWonByClient < roundWonByServer) {
                        text = (char*)"YOU LOST!";
                    } else {
                        text = (char*)"TIE!";
                    }

                    // New game message
                    gameMessage.gameState = Ended;
                    gameMessage.messageType = Informative;
                    strcpy(gameMessage.text, text);
                    gameMessage.element = serverElement;

                    isFinished = true;
                } else {
                    char *text;
                    if (wWin == 1) {
                        text = (char*)"Won";
                    } else if (wWin == -1) {
                        text = (char*)"Lost";
                    } else {
                        text = (char*)"Tie";
                    }

                    char roundText[256];
                    sprintf(roundText, "Round %d: %s", round, text);

                    // New game message
                    gameMessage.gameState = Started;
                    gameMessage.messageType = Informative;
                    strcpy(gameMessage.text, roundText);
                    gameMessage.element = serverElement;
                }


                bzero(buffer, strlen(buffer));
                memcpy(buffer, &gameMessage, BUFFER_SIZE);
                if (tcpSocketPtr->sendTo(buffer, BUFFER_SIZE) < 0) {
                    std::cout << "Error: sending data" << std::endl;
                    isFinished = true;
                    continue;
                }


            } while(!isFinished);
        } catch(...){}

        std::cout << "Connection with client was lost" << std::endl;
    }
};

int main(int argc , char *argv[]) {
    Mode mode;

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
        unsigned int portNumber;
        std::vector<std::unique_ptr<std::thread>> threads;

        std::cout << "\nInsert the port: ";
        std::cin >> portNumber;
        std::cout << "Port: " << portNumber << std::endl;

        auto serverSocketAddress = new SocketAddress(INADDR_ANY, (uint16_t)portNumber);
        std::unique_ptr<SocketAddress> serverSocketAddress_ptr (serverSocketAddress);

        TCPSocketPtr serverTCPSocketPtr = TCPSocket::CreateTCPSocket(INET);
        if (serverTCPSocketPtr == nullptr) {
            std::cout << "Error: creating socket" << std::endl;
            exit(EXIT_SUCCESS);
        }

        if (serverTCPSocketPtr->bindTo(*serverSocketAddress_ptr) < 0) {
            std::cout << "Error: binding socket" << std::endl;
            exit(EXIT_SUCCESS);
        }

        // This listenTo() call tells the socket to listen to the incoming connections.
        // The listenTo() function places all incoming connection into a backlog queue
        // until accept() call accepts the connection.
        // Here, we set the maximum size for the backlog queue to 20.
        serverTCPSocketPtr->listenTo(20);

        // The accept() call actually accepts an incoming connection
        auto clientSocketAddress = new SocketAddress();
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
                    std::cout << "Error: accepting connection" << std::endl;
                }

                printf("Server: got connection from %s port %d\n",
                       inet_ntoa(clientSocketAddress_ptr->getAsSockAddrIn()->sin_addr),
                       ntohs(clientSocketAddress_ptr->getAsSockAddrIn()->sin_port));

                std::unique_ptr<std::thread> thread_ptr(new std::thread(ClientHandler(), clientTCPSocketPtr));
                threads.push_back(std::move(thread_ptr));
            } catch (...) {
                isFinished = true;
            }

        } while (!isFinished);
    }
    else
    {
        unsigned int portNumber;
        char hostname[512];
        char buffer[BUFFER_SIZE];

        std::cout << "\nInsert the hostname: ";
        std::cin >> hostname;
        std::cout << "Hostname: " << hostname << std::endl;

        std::cout << "\nInsert the port: ";
        std::cin >> portNumber;
        std::cout << "Port: " << portNumber << std::endl;
        fflush(stdin);

        auto serverSocketAddress = new SocketAddress(hostname, (uint16_t)portNumber);
        std::unique_ptr<SocketAddress> serverSocketAddress_ptr (serverSocketAddress);

        TCPSocketPtr tcpSocketPtr = TCPSocket::CreateTCPSocket(INET);
        if (tcpSocketPtr == nullptr) {
            std::cout << "Error: creating socket" << std::endl;
            exit(EXIT_SUCCESS);
        }

        if (tcpSocketPtr->connectTo(*serverSocketAddress_ptr) < 0) {
            std::cout << "Error: connecting socket" << std::endl;
            exit(EXIT_SUCCESS);
        }


        // Wait message to start game
        bool isStarted = false;

        bzero(buffer, strlen(buffer));
        if (tcpSocketPtr->receiveFrom(buffer, BUFFER_SIZE) <= 0) {
            std::cout << "Error: reading socket" << std::endl;
            exit(EXIT_SUCCESS);
        }

        struct GameMessage gameMessage;
        memcpy(&gameMessage, buffer, BUFFER_SIZE);
        if (gameMessage.gameState == Started) {
            if (gameMessage.messageType == Informative) {
                std::cout << "-> " << gameMessage.text << std::endl;
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
            std::cout << "\n-> Instructions: " << std::endl;
            std::cout << "\t- Rock: 0" << std::endl;
            std::cout << "\t- Paper: 1" << std::endl;
            std::cout << "\t- Scissors: 2" << std::endl;
            std::cout << "\t- Lizard: 3" << std::endl;
            std::cout << "\t- Spock: 4" << std::endl;
            std::cout << "-> Your turn: ";
            std::cin >> option;
            auto clientElement = (GameElement)option;
            if (clientElement < Rock || clientElement > Spock) {
                std::cout << "Error: Invalid hand" << std::endl;
                isStarted = false;
                continue;
            }

            gameMessage.gameState = Started;
            gameMessage.messageType = Element;
            gameMessage.element = clientElement;
            strcpy(gameMessage.text, "");

            bzero(buffer, strlen(buffer));
            memcpy(buffer, &gameMessage, BUFFER_SIZE);
            if (tcpSocketPtr->sendTo(buffer, BUFFER_SIZE) < 0) {
                std::cout << "Error: sending data" << std::endl;
                isStarted = false;
                continue;
            }

            // Receive new game message
            bzero(buffer, strlen(buffer));
            if (tcpSocketPtr->receiveFrom(buffer, BUFFER_SIZE) <= 0) {
                std::cout << "Error: reading socket" << std::endl;
                isStarted = false;
                continue;
            }
            memcpy(&gameMessage, buffer, BUFFER_SIZE);

            // Process new game message
            if (gameMessage.gameState == Started) {
                if (gameMessage.messageType == Informative) {
                    std::cout << "\n-> " << stringFromGameElement(clientElement) << " vs " << stringFromGameElement(gameMessage.element) << std::endl;
                    std::cout << "-> " << gameMessage.text << std::endl;
                }

            } else if (gameMessage.gameState == Ended){
                if (gameMessage.messageType == Informative) {
                    std::cout << "\n-> " << stringFromGameElement(clientElement) << " vs " << stringFromGameElement(gameMessage.element) << std::endl;
                    std::cout << "-> " << gameMessage.text << std::endl;
                }
                isStarted = false;
            }


        } while (isStarted);
    }

    return EXIT_SUCCESS;
}