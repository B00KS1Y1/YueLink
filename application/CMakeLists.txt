file(GLOB_RECURSE YUELINK_APPLICATION_SOURCES
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

qt_add_library(YueLinkApplication STATIC
    ${YUELINK_APPLICATION_SOURCES}
)

add_library(YueLink::Application ALIAS YueLinkApplication)

target_include_directories(YueLinkApplication
    PUBLIC
        ${PROJECT_SOURCE_DIR}
)

target_link_libraries(YueLinkApplication
    PUBLIC
        YueLink::Domain
        Qt6::Core
    PRIVATE
        YueLink::Config
        Qt6::Gui
        QsLog::QsLog
)
