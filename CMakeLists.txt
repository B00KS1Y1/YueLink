cmake_minimum_required(VERSION 3.16)

project(YueLink VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
include(CTest)

option(YUELINK_BUILD_GUI "Build the Qt Quick GUI application" ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network Sql)
if(YUELINK_BUILD_GUI)
    find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2 Widgets)
    find_package(HuskarUI REQUIRED)
endif()

add_subdirectory(thirdparty/spdlog)
add_subdirectory(thirdparty/nlohmann_json)

qt_standard_project_setup(REQUIRES 6.8)

add_library(YueLinkCore STATIC
    application/chatservice.cpp
    application/chatservice.h
    application/iidentitystore.h
    discovery/ipeerdiscovery.h
    domain/chattypes.cpp
    domain/chattypes.h
    domain/networktypes.h
    storage/ichatrepository.h
    transport/ichattransport.h
)

target_include_directories(YueLinkCore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(YueLinkCore
    PUBLIC
        Qt6::Core
        Qt6::Network
    PRIVATE
        spdlog::spdlog
)

add_library(YueLinkInfrastructure STATIC
    application/runtimebootstrap.cpp
    application/runtimebootstrap.h
    config/config.h
    config/configserializer.h
    config/configstore.cpp
    config/configstore.h
    config/result.h
    discovery/udppeerdiscovery.cpp
    discovery/udppeerdiscovery.h
    logging/logging.cpp
    logging/logging.h
    protocol/wireprotocol.cpp
    protocol/wireprotocol.h
    settings/qsettingsidentitystore.cpp
    settings/qsettingsidentitystore.h
    storage/sqlitechatrepository.cpp
    storage/sqlitechatrepository.h
    transport/tcpchattransport.cpp
    transport/tcpchattransport.h
    transport/tcpfiletransfer.cpp
    utils/path.cpp
    utils/path.h
    utils/singleton.h
)

target_include_directories(YueLinkInfrastructure
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(YueLinkInfrastructure
    PUBLIC
        YueLinkCore
    PRIVATE
        Qt6::Core
        Qt6::Network
        Qt6::Sql
        spdlog::spdlog
        nlohmann_json::nlohmann_json
)

if(YUELINK_BUILD_GUI)
    set(YUELINK_QML_FILES
    qml/ChatContent.qml
    qml/FriendListPanel.qml
    qml/Main.qml
    qml/MessageComposer.qml
    qml/ProfileEditPage.qml
    qml/SettingsPage.qml
    qml/UserInfoPanel.qml
    )

    foreach(YUELINK_QML_FILE IN LISTS YUELINK_QML_FILES)
        string(REGEX REPLACE "^qml/" "" YUELINK_QML_RESOURCE_ALIAS "${YUELINK_QML_FILE}")
        set_source_files_properties("${YUELINK_QML_FILE}"
            PROPERTIES QT_RESOURCE_ALIAS "${YUELINK_QML_RESOURCE_ALIAS}"
        )
    endforeach()

    qt_add_executable(YueLink
        main.cpp
    )

    qt_add_qml_module(YueLink
        URI YueLink
        QML_FILES ${YUELINK_QML_FILES}
        SOURCES
            application/lanchatmanager.cpp
            application/lanchatmanager.h
            models/chatmessagemodel.cpp
            models/chatmessagemodel.h
            models/peerlistmodel.cpp
            models/peerlistmodel.h
            platform/desktopfilelauncher.cpp
            platform/desktopfilelauncher.h
            platform/desktopnotificationservice.cpp
            platform/desktopnotificationservice.h
            platform/ifilelauncher.h
            platform/inotificationservice.h
            settings/appsettings.cpp
            settings/appsettings.h
    )

    set_target_properties(YueLink PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
    )

    target_include_directories(YueLink
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/application
            ${CMAKE_CURRENT_SOURCE_DIR}/settings
    )

    target_link_libraries(YueLink
        PRIVATE
            YueLinkCore
            YueLinkInfrastructure
            Qt6::Core
            Qt6::Network
            Qt6::Quick
            Qt6::QuickControls2
            Qt6::Sql
            Qt6::Widgets
            HuskarUI::Basic
            spdlog::spdlog
            nlohmann_json::nlohmann_json
    )

    install(TARGETS YueLink
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

add_executable(yuelink-cli
    cli/commandparser.cpp
    cli/commandparser.h
    cli/consolecontroller.cpp
    cli/consolecontroller.h
    cli/consoleinput.cpp
    cli/consoleinput.h
    cli/consolerenderer.cpp
    cli/consolerenderer.h
    cli/main.cpp
)

set_target_properties(yuelink-cli PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_include_directories(yuelink-cli
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(yuelink-cli
    PRIVATE
        YueLinkCore
        YueLinkInfrastructure
        Qt6::Core
        Qt6::Network
        Qt6::Sql
        spdlog::spdlog
        nlohmann_json::nlohmann_json
)

if(BUILD_TESTING)
    find_package(Qt6 REQUIRED COMPONENTS Test)
    qt_add_executable(YueLinkTests
        cli/commandparser.cpp
        cli/commandparser.h
        tests/chatservicetest.cpp
    )
    target_include_directories(YueLinkTests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
    )
    target_link_libraries(YueLinkTests
        PRIVATE
            YueLinkCore
            Qt6::Core
            Qt6::Network
            Qt6::Test
    )
    add_test(NAME YueLinkTests COMMAND YueLinkTests)
endif()

install(TARGETS yuelink-cli
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
