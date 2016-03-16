cmake_minimum_required(VERSION 3.3)
project(Parking)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp Mere.h Clavier.h)
add_executable(Parking ${SOURCE_FILES})