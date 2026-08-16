file(GLOB_RECURSE YUELINK_CPP_SOURCES
    CONFIGURE_DEPENDS
    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    "*.cc"
    "*.cpp"
    "*.cxx"
    "*.h"
    "*.hh"
    "*.hpp"
    "*.hxx"
    "*.ui"
)

set(YUELINK_MAIN_SOURCES ${YUELINK_CPP_SOURCES})
list(FILTER YUELINK_MAIN_SOURCES
    INCLUDE
    REGEX "(^|/)main\\.(cc|cpp|cxx)$"
)
list(REMOVE_ITEM YUELINK_CPP_SOURCES
    ${YUELINK_MAIN_SOURCES}
)

file(GLOB_RECURSE YUELINK_QML_FILES
    CONFIGURE_DEPENDS
    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    "qml/*.js"
    "qml/*.mjs"
    "qml/*.qml"
)

file(GLOB_RECURSE YUELINK_ASSET_FILES
    CONFIGURE_DEPENDS
    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    "assets/*"
)

set(YUELINK_WINDOWS_RESOURCE_FILES ${YUELINK_ASSET_FILES})
list(FILTER YUELINK_WINDOWS_RESOURCE_FILES
    INCLUDE
    REGEX "\\.rc$"
)
list(FILTER YUELINK_ASSET_FILES
    EXCLUDE
    REGEX "\\.rc$"
)

set_source_files_properties(qml/AppTheme.qml
    PROPERTIES
        QT_QML_SINGLETON_TYPE TRUE
)

qt_add_executable(YueLink
    ${YUELINK_MAIN_SOURCES}
)

qt_add_resources(YueLink yuelink_app_assets
    PREFIX "/yuelink"
    BASE "${CMAKE_CURRENT_SOURCE_DIR}"
    FILES
        ${YUELINK_ASSET_FILES}
)

if(WIN32)
    target_sources(YueLink
        PRIVATE
            ${YUELINK_WINDOWS_RESOURCE_FILES}
    )
endif()

qt_add_qml_module(YueLink
    URI YueLink
    DEPENDENCIES
        QtQml.Models
    QML_FILES
        ${YUELINK_QML_FILES}
    SOURCES
        ${YUELINK_CPP_SOURCES}
)

set_target_properties(YueLink PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_include_directories(YueLink
    PRIVATE
        ${PROJECT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(YueLink
    PRIVATE
        YueLink::Application
        YueLink::Infrastructure
        Qt6::Core
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Widgets
        HuskarUI::Basic
        QsLog::QsLog
)

install(TARGETS YueLink
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
