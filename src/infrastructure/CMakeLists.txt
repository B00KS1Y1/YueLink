qt_add_library(YueLinkRuntime STATIC
    config/applicationconfig.cpp
    config/applicationconfig.h
    config/config.h
    config/configapi.h
    config/configbase.h
    config/configmanager.cpp
    config/configmanager.h
    config/configpolicyutils.cpp
    config/configpolicyutils_p.h
    config/configregistry.h
    config/configstore.h
    config/databaseconfig.cpp
    config/databaseconfig.h
    config/identityconfig.cpp
    config/identityconfig.h
    config/logconfig.cpp
    config/logconfig.h
    config/result.h
    config/themeconfig.cpp
    config/themeconfig.h
    logging.cpp
    logging.h
    path.cpp
    path.h
    runtimebootstrap.cpp
    runtimebootstrap.h
)

add_library(YueLink::Runtime ALIAS YueLinkRuntime)

target_include_directories(YueLinkRuntime
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_compile_definitions(YueLinkRuntime
    PRIVATE
        YUELINK_PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
        $<$<CONFIG:Debug>:YUELINK_DEBUG_BUILD>
)

target_link_libraries(YueLinkRuntime
    PUBLIC
        nlohmann_json::nlohmann_json
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
