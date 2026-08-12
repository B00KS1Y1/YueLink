qt_add_library(YueLinkConfig STATIC
    applicationconfig.cpp
    applicationconfig.h
    config.h
    configapi.h
    configbase.h
    configmanager.cpp
    configmanager.h
    configpolicyutils.cpp
    configpolicyutils_p.h
    configregistry.h
    configstore.h
    databaseconfig.cpp
    databaseconfig.h
    identityconfig.cpp
    identityconfig.h
    logconfig.cpp
    logconfig.h
    result.h
    themeconfig.cpp
    themeconfig.h
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
