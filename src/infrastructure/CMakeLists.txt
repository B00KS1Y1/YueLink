file(GLOB_RECURSE YUELINK_INFRASTRUCTURE_SOURCES
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

# Paths remains a separate foundation because the config target depends on it.
set(YUELINK_PATH_SOURCES ${YUELINK_INFRASTRUCTURE_SOURCES})
list(FILTER YUELINK_PATH_SOURCES
    INCLUDE
    REGEX "(^|/)path\\.(cc|cpp|cxx|h|hh|hpp|hxx)$"
)

list(REMOVE_ITEM YUELINK_INFRASTRUCTURE_SOURCES
    ${YUELINK_PATH_SOURCES}
)

qt_add_library(YueLinkPaths STATIC
    ${YUELINK_PATH_SOURCES}
)

add_library(YueLink::Paths ALIAS YueLinkPaths)

target_include_directories(YueLinkPaths
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_compile_definitions(YueLinkPaths
    PRIVATE
        YUELINK_PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
        $<$<CONFIG:Debug>:YUELINK_DEBUG_BUILD>
)

target_link_libraries(YueLinkPaths
    PUBLIC
        Qt6::Core
)

qt_add_library(YueLinkInfrastructure STATIC
    ${YUELINK_INFRASTRUCTURE_SOURCES}
)

add_library(YueLink::Infrastructure ALIAS YueLinkInfrastructure)

target_include_directories(YueLinkInfrastructure
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(YueLinkInfrastructure
    PUBLIC
        YueLink::Config
        YueLink::Domain
        YueLink::Paths
        Qt6::Core
        Qt6::Network
        Qt6::Sql
    PRIVATE
        QsLog::QsLog
)
