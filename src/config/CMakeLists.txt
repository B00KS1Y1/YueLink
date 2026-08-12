file(GLOB_RECURSE YUELINK_CONFIG_SOURCES
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

qt_add_library(YueLinkConfig STATIC
    ${YUELINK_CONFIG_SOURCES}
)

add_library(YueLink::Config ALIAS YueLinkConfig)

target_include_directories(YueLinkConfig
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(YueLinkConfig
    PUBLIC
        YueLinkPaths
        nlohmann_json::nlohmann_json
        Qt6::Core
)
