cmake_minimum_required(VERSION 3.16)

project(YueLink VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Network Sql)
qt_standard_project_setup(REQUIRES 6.8)

find_package(Qt6 6.8 REQUIRED COMPONENTS Quick QuickControls2 Widgets)

set(BUILD_HUSKARUI_GALLERY OFF CACHE BOOL "" FORCE)
set(BUILD_HUSKARUI_STATIC_LIBRARY OFF CACHE BOOL "" FORCE)
set(BUILD_HUSKARUI_ON_DESKTOP_PLATFORM ON CACHE BOOL "" FORCE)
set(INSTALL_HUSKARUI_IN_DEFAULT_LOCATION OFF CACHE BOOL "" FORCE)
set(HUSKARUI_INSTALL_DIRECTORY "${CMAKE_INSTALL_PREFIX}" CACHE PATH "" FORCE)
# HuskarUI intentionally uses Qt Quick private APIs.
set(QT_NO_PRIVATE_MODULE_WARNING ON)
add_subdirectory(thirdparty/HuskarUI)
add_library(HuskarUI::Basic ALIAS HuskarUIBasic)

set(QYLOG_QT_VERSION 6 CACHE STRING "" FORCE)
set(QYLOG_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(thirdparty/QyLog)
add_subdirectory(thirdparty/nlohmann_json)
add_subdirectory(thirdparty/QAutoStart EXCLUDE_FROM_ALL)

if(NOT TARGET QAutoStart::QAutoStart)
    add_library(QAutoStart::QAutoStart ALIAS QAutoStart)
endif()

add_subdirectory(config)
add_subdirectory(domain)
add_subdirectory(infrastructure)
add_subdirectory(application)
add_subdirectory(YueLink)
