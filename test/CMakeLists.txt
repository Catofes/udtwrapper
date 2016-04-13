cmake_minimum_required(VERSION 2.8)
project(udtwrapper)

find_library(UDT4_LIBS udt ../udt4-fork/src)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

include_directories(include ../udt4-fork/src)

add_executable(main main.cc)
target_link_libraries(main ${UDT4_LIBS})