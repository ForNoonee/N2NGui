cmake_minimum_required(VERSION 3.15)
project(virtual_network_gui)
set(CMAKE_PREFIX_PATH "D:\\qt6\\6.6.3\\msvc2019_64")  # 统一 Qt6 路径

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Network
    Sql
    Concurrent
    Qml
    Quick
    Core5Compat  # 正确组件名（对应 Qt5Compat 模块）
)

set(RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.h"
    "src/ui/*.cpp"
    "src/ui/*.h"
    "src/ui/*.ui"
    "src/ui/*.qml"
    "src/ui/*.qrc"  # 显式包含 .qrc
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(MyN2N ${SOURCES})

# 自动部署 Qt 运行时依赖
add_custom_command(TARGET MyN2N POST_BUILD
    COMMAND "${CMAKE_PREFIX_PATH}/bin/windeployqt.exe"
        --qmldir "${CMAKE_CURRENT_SOURCE_DIR}/src/ui"
        "$<TARGET_FILE:MyN2N>"
    COMMENT "Deploying Qt runtime dependencies..."
)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi /Oy-")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
target_link_libraries(MyN2N PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Sql
    Qt6::Concurrent
    Qt6::Qml
    Qt6::Quick
    Iphlpapi
    Ws2_32
    Qt6::Core5Compat  # 正确链接名
)

# 安装配置
install(TARGETS MyN2N
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)

install(DIRECTORY "${CMAKE_PREFIX_PATH}/plugins" DESTINATION . COMPONENT Runtime)
install(DIRECTORY "${CMAKE_PREFIX_PATH}/qml/QtQuick" DESTINATION "qml" COMPONENT Runtime)  # 新增

install(FILES
    "${CMAKE_SOURCE_DIR}/config.ini"
    DESTINATION config
)
