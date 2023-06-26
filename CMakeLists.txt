cmake_minimum_required(VERSION 2.8)
project(spi2can)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ggdb -O3 -lpthread -lstdc++")
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};/usr/local/lib")

set(PROJECT_VERSION "1.0.0")

find_library(libconfig libconfig)

add_executable(fmlx-spi2can src/main.cpp src/Spi2Can.cpp src/CanSocket.cpp src/SpidevSpi.cpp src/SysfsGpio.cpp)
target_link_libraries( 
    fmlx-spi2can 
    config++
)