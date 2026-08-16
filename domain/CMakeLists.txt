file(GLOB_RECURSE YUELINK_DOMAIN_SOURCES
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

qt_add_library(YueLinkDomain STATIC
    ${YUELINK_DOMAIN_SOURCES}
)

add_library(YueLink::Domain ALIAS YueLinkDomain)

target_include_directories(YueLinkDomain
    PUBLIC
        ${PROJECT_SOURCE_DIR}
)

target_link_libraries(YueLinkDomain
    PUBLIC
        Qt6::Core
        Qt6::Network
)
