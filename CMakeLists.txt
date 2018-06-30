cmake_minimum_required(VERSION 3.10)
project(rock_paper_scissors_lizard_spock)

set(CMAKE_CXX_STANDARD 11)

add_executable(rock_paper_scissors_lizard_spock main.cpp classes/SocketAddress.cpp classes/SocketAddress.h classes/UDPSocket.cpp classes/UDPSocket.h)