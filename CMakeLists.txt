cmake_minimum_required(VERSION 3.16)

project(YueLink VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network Quick QuickControls2 Sql Widgets)
find_package(HuskarUI REQUIRED)

add_subdirectory(thirdparty/spdlog)
add_subdirectory(thirdparty/nlohmann_json)

qt_standard_project_setup(REQUIRES 6.8)

file(GLOB YUELINK_CPP_SOURCES
    CONFIGURE_DEPENDS
    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

file(GLOB YUELINK_SOURCE_ENTRIES
    CONFIGURE_DEPENDS
    LIST_DIRECTORIES true
    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/*"
)

set(YUELINK_EXCLUDED_SOURCE_DIRECTORIES
    build
    qml
    thirdparty
)

foreach(YUELINK_SOURCE_ENTRY IN LISTS YUELINK_SOURCE_ENTRIES)
    if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${YUELINK_SOURCE_ENTRY}"
       AND NOT YUELINK_SOURCE_ENTRY MATCHES "^\\."
       AND NOT YUELINK_SOURCE_ENTRY IN_LIST YUELINK_EXCLUDED_SOURCE_DIRECTORIES)
        file(GLOB_RECURSE YUELINK_MODULE_SOURCES
            CONFIGURE_DEPENDS
            RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
            "${CMAKE_CURRENT_SOURCE_DIR}/${YUELINK_SOURCE_ENTRY}/*.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/${YUELINK_SOURCE_ENTRY}/*.h"
        )
        list(APPEND YUELINK_CPP_SOURCES ${YUELINK_MODULE_SOURCES})
    endif()
endforeach()

file(GLOB_RECURSE YUELINK_QML_FILES
    CONFIGURE_DEPENDS
    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/qml/*.qml"
)

list(SORT YUELINK_CPP_SOURCES)
list(SORT YUELINK_QML_FILES)

foreach(YUELINK_QML_FILE IN LISTS YUELINK_QML_FILES)
    string(REGEX REPLACE "^qml/" "" YUELINK_QML_RESOURCE_ALIAS "${YUELINK_QML_FILE}")
    set_source_files_properties("${YUELINK_QML_FILE}"
        PROPERTIES QT_RESOURCE_ALIAS "${YUELINK_QML_RESOURCE_ALIAS}"
    )
endforeach()

qt_add_executable(YueLink)

qt_add_qml_module(YueLink
    URI YueLink
    QML_FILES ${YUELINK_QML_FILES}
    SOURCES ${YUELINK_CPP_SOURCES}
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(YueLink PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.YueLink
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(YueLink
    PRIVATE
        Qt6::Core
        Qt6::Network
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Sql
        Qt6::Widgets
        HuskarUI::Basic
        spdlog::spdlog
        nlohmann_json::nlohmann_json
)

target_include_directories(YueLink
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/application
        ${CMAKE_CURRENT_SOURCE_DIR}/settings
)

install(TARGETS YueLink
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
