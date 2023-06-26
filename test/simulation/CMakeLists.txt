cmake_minimum_required(VERSION 2.8)
project(spi2cansimulation)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ggdb -O3 -lpthread -lstdc++")

set(PROJECT_VERSION "1.0.0")

add_executable(fmlx-spi2cansimrx src/main.cpp  ../../src/SysfsGpio.cpp  ../../src/SpidevSpi.cpp  ../../src/Spi2Can.cpp src/SimSpidevSpiTx.cpp src/SimSysfsGpio.cpp ../../src/CanSocket.cpp)