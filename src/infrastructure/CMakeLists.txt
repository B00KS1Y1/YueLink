qt_add_library(YueLinkRuntime STATIC
    config/config.h
    config/configserializer.h
    config/configstore.cpp
    config/configstore.h
    config/result.h
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

target_link_libraries(YueLinkRuntime
    PUBLIC
        Qt6::Core
    PRIVATE
        nlohmann_json::nlohmann_json
        spdlog::spdlog
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
        spdlog::spdlog
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
    PRIVATE
        YueLink::Runtime
        Qt6::Sql
        spdlog::spdlog
)
