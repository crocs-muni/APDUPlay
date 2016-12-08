cmake_minimum_required(VERSION 3.6)
project(APDUPlay)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(/usr/local/include/PCSC/ ../Shared/)

set(SOURCE_FILES Shared/status.cpp Shared/status.H Shared/globals.h Winscard/CommonFnc.cpp Winscard/CommonFnc.h
        Winscard/Interface.h Winscard/PCSCard.cpp Winscard/Resource.h Winscard/socket.cpp Winscard/socket.h
        Winscard/stdafx.cpp Winscard/stdafx.h Winscard/Winscard.cpp Winscard/Winscard.h)
add_executable(APDUPlay ${SOURCE_FILES})