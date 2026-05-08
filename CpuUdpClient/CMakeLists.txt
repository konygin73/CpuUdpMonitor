# Минимально требуемая версия CMake
cmake_minimum_required(VERSION 3.10)

project(CpuUdpClient C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_definitions(-D_DEFAULT_SOURCE)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall -Wextra")

add_executable(CpuUdpClient main.c)
