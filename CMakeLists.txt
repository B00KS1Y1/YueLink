cmake_minimum_required(VERSION 3.16)

project(YueLink VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

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
    src/core/chatservice.cpp
    src/core/chatservice.h
    src/core/chattypes.cpp
    src/core/chattypes.h
    src/core/ichatrepository.h
    src/core/ichattransport.h
    src/core/iidentitystore.h
    src/core/ipeerdiscovery.h
    src/core/networktypes.h
)

target_include_directories(YueLinkCore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(YueLinkCore
    PUBLIC
        Qt6::Core
        Qt6::Network
    PRIVATE
        spdlog::spdlog
)

add_library(YueLinkInfrastructure STATIC
    src/infrastructure/config/config.h
    src/infrastructure/config/configserializer.h
    src/infrastructure/config/configstore.cpp
    src/infrastructure/config/configstore.h
    src/infrastructure/config/result.h
    src/infrastructure/logging.cpp
    src/infrastructure/logging.h
    src/infrastructure/path.cpp
    src/infrastructure/path.h
    src/infrastructure/qsettingsidentitystore.cpp
    src/infrastructure/qsettingsidentitystore.h
    src/infrastructure/runtimebootstrap.cpp
    src/infrastructure/runtimebootstrap.h
    src/infrastructure/sqlitechatrepository.cpp
    src/infrastructure/sqlitechatrepository.h
    src/infrastructure/tcpchattransport.cpp
    src/infrastructure/tcpchattransport.h
    src/infrastructure/tcpfiletransfer.cpp
    src/infrastructure/udppeerdiscovery.cpp
    src/infrastructure/udppeerdiscovery.h
    src/infrastructure/wireprotocol.cpp
    src/infrastructure/wireprotocol.h
)

target_include_directories(YueLinkInfrastructure
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
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
        src/gui/qml/ChatContent.qml
        src/gui/qml/FriendListPanel.qml
        src/gui/qml/Main.qml
        src/gui/qml/MessageComposer.qml
        src/gui/qml/ProfileEditPage.qml
        src/gui/qml/SettingsPage.qml
        src/gui/qml/UserInfoPanel.qml
    )

    foreach(YUELINK_QML_FILE IN LISTS YUELINK_QML_FILES)
        string(REGEX REPLACE "^src/gui/qml/" "" YUELINK_QML_RESOURCE_ALIAS "${YUELINK_QML_FILE}")
        set_source_files_properties("${YUELINK_QML_FILE}"
            PROPERTIES QT_RESOURCE_ALIAS "${YUELINK_QML_RESOURCE_ALIAS}"
        )
    endforeach()

    qt_add_executable(YueLink
        src/gui/main.cpp
    )

    qt_add_resources(YueLink yuelink_app_assets
        PREFIX "/"
        BASE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui"
        FILES
            src/gui/assets/yuelink-app-icon.png
    )

    if(WIN32)
        target_sources(YueLink PRIVATE
            src/gui/assets/yuelink.rc
        )
    endif()

    qt_add_qml_module(YueLink
        URI YueLink
        QML_FILES ${YUELINK_QML_FILES}
        SOURCES
            src/gui/appsettings.cpp
            src/gui/appsettings.h
            src/gui/chatmessagemodel.cpp
            src/gui/chatmessagemodel.h
            src/gui/desktopfilelauncher.cpp
            src/gui/desktopfilelauncher.h
            src/gui/desktopnotificationservice.cpp
            src/gui/desktopnotificationservice.h
            src/gui/ifilelauncher.h
            src/gui/inotificationservice.h
            src/gui/lanchatmanager.cpp
            src/gui/lanchatmanager.h
            src/gui/peerlistmodel.cpp
            src/gui/peerlistmodel.h
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
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${CMAKE_CURRENT_SOURCE_DIR}/src/gui
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
