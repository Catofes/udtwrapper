cmake_minimum_required(VERSION 3.1)
project(udtwrapper)

find_library(UDT4_LIBS udt udt4-fork/src)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lpthread")

include_directories(include udt4-fork/src)

set(SOURCE
        src/log.cc
        src/error.cc
        src/head.cc
        src/epoll.cc
        src/config.cc
        src/udtConnection.cc
        src/tcpConnection.cc
        main.cc)

add_executable(main ${SOURCE})
target_link_libraries(main ${UDT4_LIBS})