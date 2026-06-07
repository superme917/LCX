cmake_minimum_required(VERSION 3.5)

project(LCX VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/packages.cmake)
option(LCX_BUILD_TESTS "Build LCX test programs" OFF)

add_subdirectory(src)
if(LCX_BUILD_TESTS)
    add_subdirectory(test)
endif()
