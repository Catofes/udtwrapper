cmake_minimum_required(VERSION 3.1)
project(udtwrapper)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(include)

add_executable(main main.cc)