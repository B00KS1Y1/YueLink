pragma ComponentBehavior: Bound

import QtQuick
import HuskarUI.Basic

HusWindow {
    id: root

    property string selectedPeerId: ""
    property string selectedFriendName: qsTr("等待好友上线")
    property string selectedFriendInitial: "?"
    property string selectedFriendStatus: qsTr("正在搜索局域网好友…")
    property color selectedFriendColor: "#7C8799"
    property bool selectedFriendOnline: false
    property string operationError: ""
    property string activePanel: ""
    property string currentPage: "messages"
    readonly property color navigationActiveTextColor: {
        const primary = Qt.color(AppSettings.primaryColor);
        const luminance = primary.r * 0.299 + primary.g * 0.587 + primary.b * 0.114;
        return luminance > 0.55 ? "#1F1F1F" : "#FFFFFF";
    }
    readonly property var navigationMenuTheme: Object.assign({}, HusTheme.HusMenu, {
                                                                  colorTextActive: navigationActiveTextColor
                                                              })
    readonly property int navigationCompactMode: {
        switch (AppSettings.navigationMode) {
        case "relaxed":
            return HusMenu.Mode_Relaxed;
        case "standard":
            return HusMenu.Mode_Standard;
        default:
            return HusMenu.Mode_Compact;
        }
    }
    readonly property int navigationWidth: {
        switch (navigationCompactMode) {
        case HusMenu.Mode_Relaxed:
            return 176;
        case HusMenu.Mode_Standard:
            return 80;
        default:
            return 52;
        }
    }

    captionBar.height: 52
    captionBar.showWinIcon: false
    captionBar.winTitle: qsTr("YueLink")
    captionBar.winTitleDelegate: Item {
        id: captionTitleContent

        implicitWidth: captionTitleRow.implicitWidth
        implicitHeight: 46
        width: implicitWidth
        height: 46

        Row {
            id: captionTitleRow

            anchors.verticalCenter: parent.verticalCenter
            height: parent.height
            spacing: 12

            Item {
                id: appBrand

                width: appBrandRow.implicitWidth + 16
                height: parent.height

                Rectangle {
                    anchors.fill: parent
                    radius: 6
                    color: appBrandMouseArea.containsMouse
                           ? HusTheme.Primary.colorFillTertiary
                           : "transparent"
                    Accessible.ignored: true
                }

                Row {
                    id: appBrandRow

                    anchors.centerIn: parent
                    spacing: 8

                    Image {
                        anchors.verticalCenter: parent.verticalCenter
                        width: 24
                        height: 24
                        source: "qrc:/yuelink/assets/yuelink-app-icon.png"
                        sourceSize.width: 24
                        sourceSize.height: 24
                        fillMode: Image.PreserveAspectFit
                        Accessible.ignored: true
                    }

                    HusText {
                        anchors.verticalCenter: parent.verticalCenter
                        text: qsTr("YueLink")
                        color: HusTheme.Primary.colorTextBase
                        font.pixelSize: HusTheme.Primary.fontPrimarySizeHeading5
                        font.weight: Font.Medium
                    }
                }

                MouseArea {
                    id: appBrandMouseArea

                    anchors.fill: parent
                    activeFocusOnTab: true
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    Accessible.role: Accessible.Button
                    Accessible.name: qsTr("关于 YueLink")
                    onClicked: root.openAboutWindow()
                    Keys.onReturnPressed: root.openAboutWindow()
                    Keys.onSpacePressed: root.openAboutWindow()
                }
            }

            Rectangle {
                width: 1
                height: 22
                anchors.verticalCenter: parent.verticalCenter
                color: HusTheme.Primary.colorBorderSecondary
                Accessible.ignored: true
            }

            UserInfoPanel {
                id: headerProfile

                width: 180
                height: 46
                onProfileEditRequested: root.openPanel("profile")
            }
        }

        Connections {
            target: root.captionBar

            function onWindowAgentChanged(): void {
                root.captionBar.addInteractionItem(appBrand)
                root.captionBar.addInteractionItem(headerProfile)
            }
        }

        Component.onDestruction: {
            root.captionBar.removeInteractionItem(appBrand);
            root.captionBar.removeInteractionItem(headerProfile);
        }
    }

    function applyAppearanceSettings(): void {
        HusTheme.darkMode = AppSettings.themeMode === "light"
                ? HusTheme.Light
                : AppSettings.themeMode === "system"
                  ? HusTheme.System
                  : HusTheme.Dark;
        HusTheme.installThemePrimaryColorBase(AppSettings.primaryColor);
        HusTheme.animationEnabled = AppSettings.animationsEnabled;
    }

    function openPanel(panel: string): void {
        activePanel = panel;
    }

    function openAboutWindow(): void {
        aboutWindow.show();
        aboutWindow.raise();
        aboutWindow.requestActivate();
    }

    function showOperationError(reason: string): void {
        operationError = reason;
        operationErrorTimer.restart();
    }

    function showModifiedSuccess(): void {
        successMessage.success(qsTr("修改成功"));
    }

    function refreshSelectedFriend(): void {
        if (selectedPeerId.length === 0) {
            selectedFriendName = qsTr("等待好友上线");
            selectedFriendInitial = "?";
            selectedFriendStatus = LanChat.running
                    ? qsTr("正在搜索局域网好友…")
                    : LanChat.lastError.length > 0
                      ? LanChat.lastError
                      : qsTr("局域网服务未启动");
            selectedFriendColor = "#7C8799";
            selectedFriendOnline = false;
            return;
        }

        const friend = LanChat.peerInfo(selectedPeerId);
        if (friend.peerId === undefined)
            return;

        selectedFriendName = friend.friendName;
        selectedFriendInitial = friend.initial;
        selectedFriendStatus = friend.statusText;
        selectedFriendColor = friend.avatarColor;
        selectedFriendOnline = friend.online;
    }

    function selectFriend(peerId: string): void {
        if (!LanChat.selectPeer(peerId))
            return;

        operationError = "";
        selectedPeerId = peerId;
        refreshSelectedFriend();
    }

    function startNetworkService(): void {
        operationError = "";
        if (!LanChat.start())
            showOperationError(LanChat.lastError.length > 0
                               ? LanChat.lastError
                               : qsTr("无法启动局域网服务"));
    }

    function refreshNetworkDiscovery(): void {
        operationError = "";
        if (!LanChat.refreshPeers())
            showOperationError(LanChat.lastError.length > 0
                               ? LanChat.lastError
                               : qsTr("无法刷新局域网好友"));
    }

    Component.onCompleted: {
        applyAppearanceSettings();
        HusTheme.installThemePrimaryFontSizeBase(16);
        LanChat.setNotificationsEnabled(AppSettings.notificationsEnabled);
    }

    Connections {
        target: AppSettings

        function onSettingsChanged(): void {
            root.applyAppearanceSettings();
            LanChat.setNotificationsEnabled(AppSettings.notificationsEnabled);
        }
    }

    Connections {
        target: LanChat

        function onPeerDiscovered(peerId: string): void {
            if (root.selectedPeerId.length === 0)
                root.selectFriend(peerId);
        }

        function onPeerUpdated(peerId: string): void {
            if (peerId === root.selectedPeerId)
                root.refreshSelectedFriend();
        }

        function onRunningChanged(): void {
            root.refreshSelectedFriend();
        }

        function onLastErrorChanged(): void {
            if (root.selectedPeerId.length === 0)
                root.refreshSelectedFriend();
        }

        function onSendFailed(peerId: string, reason: string): void {
            if (peerId === root.selectedPeerId)
                root.showOperationError(reason);
        }

        function onFileTransferFailed(peerId: string, reason: string): void {
            if (peerId === root.selectedPeerId)
                root.showOperationError(reason);
        }

        function onOperationFailed(reason: string): void {
            root.showOperationError(reason);
        }

        function onNotificationActivated(peerId: string): void {
            root.activePanel = "";
            root.show();
            root.raise();
            root.requestActivate();
            root.selectFriend(peerId);
        }
    }

    Timer {
        id: operationErrorTimer

        interval: 5000
        onTriggered: root.operationError = ""
    }

    width: 1180
    height: 760
    minimumWidth: 960
    minimumHeight: 620
    visible: true
    title: LanChat.totalUnreadCount > 0
           ? qsTr("YueLink（%1 条未读）").arg(LanChat.totalUnreadCount)
           : qsTr("YueLink")

    onActiveChanged: {
        if (active && selectedPeerId.length > 0)
            LanChat.markConversationRead(selectedPeerId);
    }

    Item {
        anchors.fill: parent
        anchors.topMargin: root.captionBar.height

        Rectangle {
            id: navigationRail

            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: root.navigationWidth
            color: HusTheme.Primary.colorBgContainer

            HusMenu {
                id: primaryNavigation

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: settingsNavigation.top
                anchors.margins: 6
                compactMode: root.navigationCompactMode
                themeSource: root.navigationMenuTheme
                showToolTip: root.navigationCompactMode !== HusMenu.Mode_Relaxed
                defaultMenuWidth: 176
                defaultMenuTopPadding: 10
                defaultMenuBottomPadding: 10
                defaultSelectedKeys: ["messages"]
                initModel: [
                    {
                        key: "messages",
                        label: qsTr("消息"),
                        shortLabel: qsTr("消息"),
                        iconSource: HusIcon.MessageOutlined
                    },
                    {
                        key: "contacts",
                        label: qsTr("联系人"),
                        shortLabel: qsTr("联系人"),
                        iconSource: HusIcon.ContactsOutlined
                    },
                ]
                onClickMenu: (deep, key) => {
                    if (deep !== 0)
                        return;

                    root.currentPage = key;
                }
            }

            HusMenu {
                id: settingsNavigation

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 6
                height: implicitHeight
                compactMode: root.navigationCompactMode
                themeSource: root.navigationMenuTheme
                showToolTip: root.navigationCompactMode !== HusMenu.Mode_Relaxed
                defaultMenuWidth: 176
                defaultMenuTopPadding: 10
                defaultMenuBottomPadding: 10
                initModel: [
                    {
                        key: "settings",
                        label: qsTr("设置"),
                        shortLabel: qsTr("设置"),
                        iconSource: HusIcon.SettingOutlined
                    }
                ]
                onClickMenu: (deep, key) => {
                    if (deep === 0 && key === "settings")
                        root.openPanel("settings");
                }
            }
        }

        Rectangle {
            id: navigationDivider

            anchors.left: navigationRail.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 1
            color: HusTheme.Primary.colorBorderSecondary
        }

        Rectangle {
            id: friendPanel

            anchors.left: navigationDivider.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 304
            color: HusTheme.Primary.colorBgContainer

            FriendListPanel {
                id: friendList

                anchors.fill: parent
                selectedPeerId: root.selectedPeerId
                contactsMode: root.currentPage === "contacts"
                onFriendSelected: peerId => root.selectFriend(peerId)
                onNetworkStartRequested: root.startNetworkService()
                onNetworkRefreshRequested: root.refreshNetworkDiscovery()
            }
        }

        Rectangle {
            id: mainDivider

            anchors.left: friendPanel.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 1
            color: HusTheme.Primary.colorBorderSecondary
        }

        Rectangle {
            anchors.left: mainDivider.right
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            color: HusTheme.Primary.colorFillQuaternary

            ChatContent {
                id: chatContent

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: messageComposer.top
                friendName: root.selectedFriendName
                friendInitial: root.selectedFriendInitial
                friendStatus: root.selectedFriendStatus
                friendColor: root.selectedFriendColor
                friendOnline: root.selectedFriendOnline
                peerSelected: root.selectedPeerId.length > 0
                onCancelFileRequested: messageId => {
                    if (root.selectedPeerId.length > 0)
                        LanChat.cancelFileTransfer(root.selectedPeerId, messageId);
                }
                onOpenFileRequested: filePath => LanChat.openFile(filePath)
                onRevealFileRequested: filePath => LanChat.revealFile(filePath)
            }

            MessageComposer {
                id: messageComposer

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 176
                peerId: root.selectedPeerId
                sendEnabled: root.selectedFriendOnline
                errorText: root.operationError
                onFilesSelected: fileUrls => {
                    if (root.selectedPeerId.length > 0)
                        LanChat.sendFiles(root.selectedPeerId, fileUrls);
                }
            }
        }
    }

    AboutWindow {
        id: aboutWindow

        transientParent: root
    }

    HusMessage {
        id: successMessage

        anchors.fill: parent
        z: 1000
    }

    Loader {
        active: root.activePanel === "profile"
        sourceComponent: profilePanelComponent
    }

    Loader {
        active: root.activePanel === "settings"
        sourceComponent: settingsPanelComponent
    }

    Component {
        id: profilePanelComponent

        Item {
            HusModal {
                id: profileModal

                width: Math.min(540, root.width - 64)
                height: Math.min(612, root.height - 24)
                closable: false
                maskClosable: true
                contentDelegate: Item {
                    height: profileModal.height

                    HusText {
                        id: profileTitle

                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.leftMargin: 24
                        anchors.topMargin: 18
                        text: qsTr("编辑个人资料")
                        color: HusTheme.Primary.colorTextBase
                        font.pixelSize: HusTheme.Primary.fontPrimarySizeHeading4
                        font.weight: Font.Medium
                    }

                    HusIconButton {
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.rightMargin: 10
                        anchors.topMargin: 10
                        width: 32
                        height: 32
                        padding: 0
                        type: HusButton.Type_Text
                        iconSource: HusIcon.CloseOutlined
                        iconSize: 17
                        contentDescription: qsTr("关闭个人资料编辑")
                        onClicked: profileModal.close()
                    }

                    ProfileEditPage {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: profileTitle.bottom
                        anchors.bottom: parent.bottom
                        anchors.topMargin: 2
                        onCloseRequested: profileModal.close()
                        onSaved: {
                            profileModal.close();
                            root.showModifiedSuccess();
                        }
                    }
                }
                Component.onCompleted: open()
                onClosed: root.activePanel = ""
            }
        }
    }

    Component {
        id: settingsPanelComponent

        Item {
            HusDrawer {
                id: settingsDrawer

                title: qsTr("设置")
                drawerSize: Math.min(560, root.width - 80)
                maskClosable: true
                closePosition: HusDrawer.Position_End
                contentDelegate: SettingsPage {
                    onCloseRequested: settingsDrawer.close()
                    onSaved: {
                        settingsDrawer.close();
                        root.showModifiedSuccess();
                    }
                }
                Component.onCompleted: open()
                onClosed: root.activePanel = ""
            }
        }
    }
}
