cmake_minimum_required(VERSION 3.6)
project(APDUPlay)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(/usr/local/include/PCSC/ ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCE_FILES Shared/status.cpp Shared/status.H Shared/globals.h Winscard/CommonFnc.cpp Winscard/CommonFnc.h
        Winscard/Resource.h Winscard/stdafx.cpp Winscard/stdafx.h
        Winscard/Winscard.cpp Winscard/Winscard.h Iniparser/iniparser_all.h)

add_library(pcsclite SHARED ${SOURCE_FILES})