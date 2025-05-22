cmake_minimum_required(VERSION 3.15)
project(virtual_network_gui)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Network Sql Concurrent)  # 添加Qt SQL查找

include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5Network_INCLUDE_DIRS})
include_directories(${Qt5Sql_INCLUDE_DIRS})  # 添加Qt SQL包含目录
set(RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

# 添加Qt的所有包含目录
include_directories(${Qt5_INCLUDE_DIRS})

file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.h"
    "src/ui/*.cpp"
    "src/ui/*.h"
    "src/ui/*.qrc"
    "src/ui/*.ui"
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(MyN2N ${SOURCES})
install(TARGETS MyN2N
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)

# 安装 Qt 插件（关键！解决依赖问题）
install(DIRECTORY "${CMAKE_PREFIX_PATH}/plugins" DESTINATION . COMPONENT Runtime)

# 安装配置文件
install(FILES
    "${CMAKE_SOURCE_DIR}/config.ini"
    DESTINATION config
)

# 安装文档

set_target_properties(MyN2N PROPERTIES
    LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\" /SUBSYSTEM:WINDOWS"
)

target_link_libraries(MyN2N PRIVATE
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
include(InstallRequiredSystemLibraries)  # 自动包含系统运行库
include(CPack)

# 必须设置的基础变量
set(CPACK_PACKAGE_NAME "VirtualNetworkGUI")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_VENDOR "YourCompany")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "VPN Client Management Tool")

# Debug 模式特殊配置
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CPACK_PACKAGE_NAME "${CPACK_PACKAGE_NAME}-Debug")
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-Debug")
endif()

# NSIS 生成器配置
set(CPACK_GENERATOR "NSIS")
set(CPACK_NSIS_PACKAGE_NAME "${CPACK_PACKAGE_NAME}")
set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_NAME}")
include(CPack)
