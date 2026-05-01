cmake_minimum_required(VERSION 3.5)

project(LCX VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(d3party/ElaWidgetTools/)
include_directories(d3party/QCloudMusicApi/)
include_directories(${PROJECT_SOURCE_DIR}/src)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network AxContainer)
set(LIBS
    Qt6::Widgets
    Qt6::Network
    Qt6::AxContainer
    ${CMAKE_SOURCE_DIR}/d3party/ElaWidgetTools/lib/ElaWidgetTools.lib
    ${CMAKE_SOURCE_DIR}/d3party/QCloudMusicApi/lib/QCloudMusicApi.lib
)
add_subdirectory(src)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(LCX)
endif()
