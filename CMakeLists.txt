cmake_minimum_required(VERSION 3.10)
project(ServerClient)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS -pthread)

add_subdirectory(Server)
add_subdirectory(Client)
