//
// Created by Saul Moreno Abril on 04/07/2018.
//

#ifndef ROCK_PAPER_SCISSORS_LIZARD_SPOCK_SOCKETADDRESSFAMILY_H
#define ROCK_PAPER_SCISSORS_LIZARD_SPOCK_SOCKETADDRESSFAMILY_H

#ifdef __APPLE__
#include <netdb.h>

#elif _WIN64

#endif

enum SocketAddressFamily {
    INET = AF_INET,
    INET6 = AF_INET6
};


#endif //ROCK_PAPER_SCISSORS_LIZARD_SPOCK_SOCKETADDRESSFAMILY_H
