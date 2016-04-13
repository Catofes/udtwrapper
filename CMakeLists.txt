cmake_minimum_required(VERSION 2.8)
project(udtwrapper)

find_library(UDT4_LIBS udt udt4-fork/src)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

include_directories(include udt4-fork/src)

set(UDT4WRAPPERLIB
        src/log.cc
        src/error.cc
        src/head.cc
        src/epoll.cc
        src/config.cc
        src/sessionManager.cc
        src/session.cc
        src/udtConnection.cc
        src/tcpConnection.cc
        )

add_library(udt4wrapper SHARED ${UDT4WRAPPERLIB})
target_link_libraries(udt4wrapper ${UDT4_LIBS})

add_executable(server server.cc)
target_link_libraries(server udt4wrapper ${UDT4_LIBS})
add_executable(client client.cc)
target_link_libraries(client udt4wrapper ${UDT4_LIBS})

add_subdirectory(test)