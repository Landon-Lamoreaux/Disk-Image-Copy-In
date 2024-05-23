cmake_minimum_required(VERSION 3.28)
project(dicpi)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

file(GLOB C_SOURCES *.c)

add_executable(dicpi main.cpp
                bitmap.c
                driver.c)

target_sources(dicpi PRIVATE ${C_SOURCES})