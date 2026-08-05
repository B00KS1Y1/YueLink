qt_add_executable(YueLink
    main.cpp
)

qt_add_resources(YueLink yuelink_app_assets
    PREFIX "/yuelink"
    BASE "${CMAKE_CURRENT_SOURCE_DIR}"
    FILES
        assets/yuelink-app-icon.png
)

if(WIN32)
    target_sources(YueLink
        PRIVATE
            assets/yuelink.rc
    )
endif()

qt_add_qml_module(YueLink
    URI YueLink
    QML_FILES
        qml/AboutWindow.qml
        qml/ChatContent.qml
        qml/FriendListPanel.qml
        qml/GroupCreateDialog.qml
        qml/GroupInfoPanel.qml
        qml/Main.qml
        qml/MessageComposer.qml
        qml/ProfileEditPage.qml
        qml/SettingsPage.qml
        qml/UserInfoPanel.qml
    SOURCES
        appsettings.cpp
        appsettings.h
        chatmessagemodel.cpp
        chatmessagemodel.h
        conversationlistviewmodel.cpp
        conversationlistviewmodel.h
        conversationviewmodel.cpp
        conversationviewmodel.h
        desktopfilelauncher.cpp
        desktopfilelauncher.h
        desktopintegration.cpp
        desktopintegration.h
        desktopnotificationservice.cpp
        desktopnotificationservice.h
        ifilelauncher.h
        inotificationservice.h
        lanchatmanager.cpp
        lanchatmanager.h
        peerlistviewmodel.cpp
        peerlistviewmodel.h
        sidebaritemmodel.cpp
        sidebaritemmodel.h
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
        ${PROJECT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(YueLink
    PRIVATE
        YueLink::Application
        YueLink::Network
        YueLink::Persistence
        YueLink::Runtime
        Qt6::Core
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Widgets
        HuskarUI::Basic
        spdlog::spdlog
)

install(TARGETS YueLink
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
