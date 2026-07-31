qt_add_library(YueLinkDomain STATIC
    chattypes.cpp
    chattypes.h
    ichatrepository.h
    ichattransport.h
    ipeerdiscovery.h
    networktypes.h
)

add_library(YueLink::Domain ALIAS YueLinkDomain)

target_include_directories(YueLinkDomain
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(YueLinkDomain
    PUBLIC
        Qt6::Core
        Qt6::Network
)
