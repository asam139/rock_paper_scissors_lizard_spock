#include <iostream>
#include <sys/socket.h>
#include <netdb.h>

enum Mode : int8_t {
    ModeServer = 0,
    ModeClient = 1
};

int main() {
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
        return 0;
    }
    mode = static_cast<Mode>(input);

    if (mode == ModeServer)
    {

    }
    else
    {

    }

    {



    }


    return 0;
}