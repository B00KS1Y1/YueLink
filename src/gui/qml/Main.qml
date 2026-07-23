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

    function showOperationError(reason: string): void {
        operationError = reason;
        operationErrorTimer.restart();
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

    function sendMessage(message: string): void {
        if (selectedPeerId.length > 0)
            LanChat.sendMessage(selectedPeerId, message);
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
            if (root.selectedPeerId.length === 0)
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
            id: friendPanel

            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 320
            color: HusTheme.Primary.colorBgContainer

            UserInfoPanel {
                id: userInfo

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                height: 92
                onProfileEditRequested: root.openPanel("profile")
                onSettingsRequested: root.openPanel("settings")
            }

            FriendListPanel {
                id: friendList

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: userInfo.bottom
                anchors.bottom: parent.bottom
                selectedPeerId: root.selectedPeerId
                onFriendSelected: peerId => root.selectFriend(peerId)
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
                sendEnabled: root.selectedFriendOnline
                errorText: root.operationError
                onSendRequested: message => root.sendMessage(message)
                onFilesSelected: fileUrls => {
                    if (root.selectedPeerId.length > 0)
                        LanChat.sendFiles(root.selectedPeerId, fileUrls);
                }
            }
        }
    }

    Loader {
        active: root.activePanel.length > 0
        sourceComponent: root.activePanel === "profile"
                         ? profilePanelComponent
                         : settingsPanelComponent
    }

    Component {
        id: profilePanelComponent

        Item {
            HusDrawer {
                id: profileDrawer

                title: qsTr("编辑个人信息")
                drawerSize: Math.min(520, root.width - 80)
                maskClosable: true
                closePosition: HusDrawer.Position_End
                contentDelegate: ProfileEditPage {
                    onCloseRequested: profileDrawer.close()
                    onSaved: profileDrawer.close()
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
                    onSaved: settingsDrawer.close()
                }
                Component.onCompleted: open()
                onClosed: root.activePanel = ""
            }
        }
    }
}
