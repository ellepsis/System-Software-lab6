cmake_minimum_required(VERSION 3.1)
project(lab6)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c99 -pthread")
set(CMAKE_EXE_LINKER_FLAGS  "-lpthread -lrt -pthread" )

add_executable(lab6 main.c fileinfo.h fileinfo.c overal_functions.c main.h)

add_executable(lab6client client.h client.c overal_functions.c)