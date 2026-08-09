qt_add_library(YueLinkApplication STATIC
    chatcoordinator.cpp
    chatcoordinator.h
    conversationstore.cpp
    conversationstore.h
    transfercoordinator.cpp
    transfercoordinator.h
)

add_library(YueLink::Application ALIAS YueLinkApplication)

target_include_directories(YueLinkApplication
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(YueLinkApplication
    PUBLIC
        YueLink::Domain
        Qt6::Core
    PRIVATE
        YueLink::Runtime
        Qt6::Gui
        QsLog::QsLog
)
