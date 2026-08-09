cmake_minimum_required(VERSION 3.16)

project(YueLink VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Network Sql)
qt_standard_project_setup(REQUIRES 6.8)

find_package(Qt6 6.8 REQUIRED COMPONENTS Quick QuickControls2 Widgets)
find_package(HuskarUI REQUIRED)

# HuskarUI 的 qmldir 使用未命名空间的动态插件目标名，但当前安装包只导出
# HuskarUI::huskaruibasicplugin，且未导出 Impl 插件目标。为 Qt 的 QML 导入扫描器
# 提供兼容目标；动态插件仍由 QML 引擎从 HuskarUI_QML_IMPORT_PATH 按需加载。
if(TARGET HuskarUI::huskaruibasicplugin AND NOT TARGET huskaruibasicplugin)
    add_library(huskaruibasicplugin ALIAS HuskarUI::huskaruibasicplugin)
endif()

if(NOT TARGET huskaruiimplplugin)
    add_library(huskaruiimplplugin MODULE IMPORTED)
endif()

add_subdirectory(thirdparty/QsLog)
add_subdirectory(thirdparty/nlohmann_json)

add_subdirectory(src/domain)
add_subdirectory(src/infrastructure)
add_subdirectory(src/application)
add_subdirectory(src/YueLink)
