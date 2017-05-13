cmake_minimum_required(VERSION 3.5)
project(WinscardTests)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wl,--no-as-needed -ldl")

include_directories(/usr/local/include/PCSC/ ${CMAKE_CURRENT_SOURCE_DIR})


set(SOURCE_FILES
        catch.hpp
        main.cpp)

add_executable(WinscardTests ${SOURCE_FILES})