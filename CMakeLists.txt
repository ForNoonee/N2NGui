cmake_minimum_required(VERSION 3.19)

project("ForNoonee")

find_package(Qt5 COMPONENTS Core Gui Widgets Network REQUIRED)
add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 指定源文件和头文件
set(SOURCES
    "Src/main.cpp"
    "Src/Project/MainWindow.cpp"
)

set(HEADERS
    "Src/Project/FMainWindowForm.h"
    "Src/Project/MainWindow.h"
)



# 将源代码添加到此项目的可执行文件
add_executable(ForNoonee ${SOURCES} ${HEADERS})

# 链接 Qt 库
target_link_libraries(ForNoonee Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Network ws2_32)
# 设置目标属性
set_target_properties(ForNoonee PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)