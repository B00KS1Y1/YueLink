qt_add_library(YueLinkPaths STATIC
    path.cpp
    path.h
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

qt_add_library(YueLinkRuntime STATIC
    logging.cpp
    logging.h
    runtimebootstrap.cpp
    runtimebootstrap.h
)

add_library(YueLink::Runtime ALIAS YueLinkRuntime)

target_include_directories(YueLinkRuntime
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(YueLinkRuntime
    PUBLIC
        YueLink::Config
        YueLink::Paths
        Qt6::Core
    PRIVATE
        QsLog::QsLog
)

qt_add_library(YueLinkNetwork STATIC
    tcpchattransport.cpp
    tcpchattransport.h
    tcpfiletransfer.cpp
    udppeerdiscovery.cpp
    udppeerdiscovery.h
    wireprotocol.cpp
    wireprotocol.h
)

add_library(YueLink::Network ALIAS YueLinkNetwork)

target_include_directories(YueLinkNetwork
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(YueLinkNetwork
    PUBLIC
        YueLink::Domain
        Qt6::Core
        Qt6::Network
    PRIVATE
        YueLink::Runtime
        QsLog::QsLog
)

qt_add_library(YueLinkPersistence STATIC
    sqlitechatrepository.cpp
    sqlitechatrepository.h
)

add_library(YueLink::Persistence ALIAS YueLinkPersistence)

target_include_directories(YueLinkPersistence
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(YueLinkPersistence
    PUBLIC
        YueLink::Domain
        Qt6::Core
        Qt6::Sql
    PRIVATE
        YueLink::Runtime
        QsLog::QsLog
)
