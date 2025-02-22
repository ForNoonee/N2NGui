cmake_minimum_required(VERSION 3.15)
project(VirtualNetworkGUI)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Network Sql Concurrent)  # 添加Qt SQL查找

include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5Network_INCLUDE_DIRS})
include_directories(${Qt5Sql_INCLUDE_DIRS})  # 添加Qt SQL包含目录

# 添加Qt的所有包含目录
include_directories(${Qt5_INCLUDE_DIRS})

file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.h"
    "src/ui/*.cpp"
    "src/ui/*.h"
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(virtual_network_gui ${SOURCES})

set_target_properties(virtual_network_gui PROPERTIES
    LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\" /SUBSYSTEM:WINDOWS"
)

target_link_libraries(virtual_network_gui PRIVATE
    Qt5::Core
    Qt5::Widgets
    Qt5::Network
    Qt5::Sql
    Qt5::Concurrent  # 添加Qt SQL链接
    Iphlpapi
    Ws2_32
)
# 设置 CMake 的前缀路径，以便找到 Qt 的安装位置
set(CMAKE_PREFIX_PATH "C:/Qt/5.15.2/msvc2019_64")

