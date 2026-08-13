pragma ComponentBehavior: Bound

import QtQuick
import HuskarUI.Basic

HusWindow {
    id: root

    property string selectedConversationId: ""
    property string selectedConversationTitle: qsTr("选择一个会话")
    property string selectedConversationInitial: "?"
    property string selectedConversationStatus: qsTr("从左侧选择联系人或群聊")
    property color selectedConversationColor: "#7C8799"
    property bool selectedConversationOnline: false
    property string selectedConversationKind: ""
    property string selectedConversationPeerId: ""
    property int selectedConversationMemberCount: 0
    property int selectedConversationOnlineCount: 0
    property string activePanel: ""
    property string currentPage: "messages"
    readonly property color navigationActiveBackgroundColor: AppTheme.accentSoftStrong
    readonly property color navigationHoverBackgroundColor: AppTheme.hover
    readonly property color navigationActiveTextColor: AppTheme.dark
                                                        ? AppTheme.accent
                                                        : AppTheme.textPrimary
    readonly property var navigationMenuTheme: Object.assign({}, HusTheme.HusMenu, {
                                                                  colorTextActive: navigationActiveTextColor,
                                                                  colorBgActive: navigationActiveBackgroundColor,
                                                                  colorBgHover: navigationHoverBackgroundColor,
                                                                  radiusMenuBg: 10
                                                              })
    readonly property var settingsMenuTheme: Object.assign({}, navigationMenuTheme, {
                                                                colorTextActive: HusTheme.Primary.colorPrimary,
                                                                colorBgActive: "transparent"
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
    readonly property int conversationListWidth: Math.round(
                                                      Math.min(320,
                                                               Math.max(280,
                                                                        width * 0.26)))

    captionBar.height: 56
    captionBar.color: AppTheme.canvas
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
                    radius: AppTheme.radiusSmall
                    color: appBrandMouseArea.containsMouse
                           ? AppTheme.hover
                           : "transparent"
                    border.width: appBrandMouseArea.activeFocus ? 1 : 0
                    border.color: AppTheme.accent
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
                        color: AppTheme.textPrimary
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
                color: AppTheme.divider
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
        operationMessage.error(reason, 5000);
    }

    function showModifiedSuccess(): void {
        operationMessage.success(qsTr("修改成功"));
    }

    function refreshSelectedConversation(): void {
        if (selectedConversationId.length === 0) {
            selectedConversationTitle = qsTr("选择一个会话");
            selectedConversationInitial = "?";
            selectedConversationStatus = LanChat.running
                    ? qsTr("正在搜索局域网联系人…")
                    : LanChat.lastError.length > 0
                      ? LanChat.lastError
                      : qsTr("局域网服务未启动");
            selectedConversationColor = "#7C8799";
            selectedConversationOnline = false;
            selectedConversationKind = "";
            selectedConversationPeerId = "";
            selectedConversationMemberCount = 0;
            selectedConversationOnlineCount = 0;
            return;
        }

        const conversation = LanChat.conversationInfo(selectedConversationId);
        if (conversation.itemId === undefined)
            return;

        selectedConversationTitle = conversation.title;
        selectedConversationInitial = conversation.initial;
        selectedConversationStatus = conversation.statusText;
        selectedConversationColor = conversation.avatarColor;
        selectedConversationOnline = conversation.online;
        selectedConversationKind = conversation.itemKind;
        selectedConversationPeerId = conversation.peerId;
        selectedConversationMemberCount = conversation.memberCount;
        selectedConversationOnlineCount = conversation.onlineCount;
    }

    function selectConversation(conversationId: string): void {
        if (!LanChat.selectConversation(conversationId))
            return;

        selectedConversationId = conversationId;
        refreshSelectedConversation();
    }

    function startNetworkService(): void {
        if (!LanChat.start())
            showOperationError(LanChat.lastError.length > 0
                               ? LanChat.lastError
                               : qsTr("无法启动局域网服务"));
    }

    function refreshNetworkDiscovery(): void {
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
            if (root.selectedConversationId.length === 0)
                root.selectConversation("direct:" + peerId);
        }

        function onPeerUpdated(peerId: string): void {
            if (peerId === root.selectedConversationPeerId)
                root.refreshSelectedConversation();
        }

        function onRunningChanged(): void {
            root.refreshSelectedConversation();
        }

        function onLastErrorChanged(): void {
            if (root.selectedConversationId.length === 0)
                root.refreshSelectedConversation();
        }

        function onSendFailed(conversationId: string, reason: string): void {
            if (conversationId === root.selectedConversationId)
                root.showOperationError(reason);
        }

        function onFileTransferFailed(conversationId: string, reason: string): void {
            if (conversationId === root.selectedConversationId)
                root.showOperationError(reason);
        }

        function onOperationFailed(reason: string): void {
            root.showOperationError(reason);
        }

        function onNotificationActivated(conversationId: string): void {
            root.activePanel = "";
            root.show();
            root.raise();
            root.requestActivate();
            root.selectConversation(conversationId);
        }
    }

    Connections {
        target: LanChat.conversations

        function onModelReset(): void {
            root.refreshSelectedConversation();
        }
    }

    width: 1180
    height: 760
    minimumWidth: 960
    minimumHeight: 620
    visible: true
    title: LanChat.totalUnreadCount > 0
           ? qsTr("YueLink（%1 条未读）").arg(LanChat.totalUnreadCount)
           : qsTr("YueLink")
    color: AppTheme.canvas

    onActiveChanged: {
        if (active && selectedConversationId.length > 0)
            LanChat.markConversationRead(selectedConversationId);
    }

    Rectangle {
        id: applicationBackground

        anchors.fill: parent
        color: AppTheme.canvas
        Accessible.ignored: true
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: root.captionBar.height - 1
        height: 1
        color: AppTheme.divider
        Accessible.ignored: true
    }

    Item {
        anchors.fill: parent
        anchors.topMargin: root.captionBar.height

        Rectangle {
            id: navigationRail

            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.leftMargin: 12
            anchors.topMargin: 12
            anchors.bottomMargin: 12
            width: root.navigationWidth
            radius: AppTheme.radiusLarge
            color: AppTheme.navigationSurface
            border.width: 1
            border.color: AppTheme.border

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
                themeSource: root.settingsMenuTheme
                showToolTip: root.navigationCompactMode !== HusMenu.Mode_Relaxed
                defaultMenuWidth: 176
                defaultMenuTopPadding: 10
                defaultMenuBottomPadding: 10
                initModel: [
                    {
                        key: "refreshContacts",
                        label: qsTr("刷新好友"),
                        shortLabel: qsTr("刷新"),
                        iconSource: HusIcon.ReloadOutlined
                    },
                    {
                        key: "settings",
                        label: qsTr("设置"),
                        shortLabel: qsTr("设置"),
                        iconSource: HusIcon.SettingOutlined
                    }
                ]
                onClickMenu: (deep, key) => {
                    if (deep !== 0)
                        return;

                    if (key === "refreshContacts") {
                        if (LanChat.running)
                            root.refreshNetworkDiscovery();
                        else
                            root.startNetworkService();
                    } else if (key === "settings") {
                        root.openPanel("settings");
                    }
                }
            }
        }

        Rectangle {
            id: friendPanel

            anchors.left: navigationRail.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.leftMargin: 12
            anchors.topMargin: 12
            anchors.bottomMargin: 12
            width: root.conversationListWidth
            radius: AppTheme.radiusLarge
            color: AppTheme.surface
            border.width: 1
            border.color: AppTheme.border

            FriendListPanel {
                id: friendList

                anchors.fill: parent
                selectedConversationId: root.selectedConversationId
                contactsMode: root.currentPage === "contacts"
                onConversationSelected: conversationId =>
                                        root.selectConversation(conversationId)
                onCreateGroupRequested: {
                    LanChat.peerSearchText = "";
                    root.openPanel("createGroup");
                }
            }
        }

        Rectangle {
            id: conversationPanel

            anchors.left: friendPanel.right
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            anchors.topMargin: 12
            anchors.bottomMargin: 12
            radius: AppTheme.radiusLarge
            color: AppTheme.surfaceSubtle
            border.width: 1
            border.color: AppTheme.border

            ChatContent {
                id: chatContent

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: messageComposer.top
                anchors.bottomMargin: 8
                conversationTitle: root.selectedConversationTitle
                conversationInitial: root.selectedConversationInitial
                conversationStatus: root.selectedConversationStatus
                conversationColor: root.selectedConversationColor
                conversationOnline: root.selectedConversationOnline
                conversationSelected: root.selectedConversationId.length > 0
                conversationKind: root.selectedConversationKind
                memberCount: root.selectedConversationMemberCount
                onlineCount: root.selectedConversationOnlineCount
                onGroupInfoRequested: root.openPanel("groupInfo")
                onCancelFileRequested: messageId => {
                    if (root.selectedConversationId.length > 0)
                        LanChat.cancelFileTransfer(root.selectedConversationId,
                                                   messageId);
                }
                onAcceptFileRequested: messageId => {
                    if (root.selectedConversationId.length > 0)
                        LanChat.acceptFileTransfer(root.selectedConversationId,
                                                   messageId);
                }
                onOpenFileRequested: filePath => LanChat.openFile(filePath)
                onRevealFileRequested: filePath => LanChat.revealFile(filePath)
            }

            MessageComposer {
                id: messageComposer

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.leftMargin: 14
                anchors.rightMargin: 14
                anchors.bottomMargin: 14
                height: implicitHeight
                conversationId: root.selectedConversationId
                sendEnabled: root.selectedConversationId.length > 0
                             && (root.selectedConversationKind === "group"
                                 || root.selectedConversationOnline)
                filesEnabled: root.selectedConversationKind === "direct"
                              && root.selectedConversationOnline
                onImagesSelected: imageUrls => {
                    if (root.selectedConversationId.length > 0)
                        LanChat.sendImages(root.selectedConversationId, imageUrls);
                }
                onFilesSelected: fileUrls => {
                    if (root.selectedConversationId.length > 0)
                        LanChat.sendFiles(root.selectedConversationId, fileUrls);
                }
            }
        }
    }

    AboutWindow {
        id: aboutWindow

        transientParent: root
    }

    HusMessage {
        id: operationMessage

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

    Loader {
        anchors.fill: parent
        active: root.activePanel === "createGroup"
        sourceComponent: groupCreateComponent
    }

    Loader {
        anchors.fill: parent
        active: root.activePanel === "groupInfo"
        sourceComponent: groupInfoComponent
    }

    Component {
        id: groupCreateComponent

        GroupCreateDialog {
            anchors.fill: parent
            onClosed: root.activePanel = ""
            onGroupCreated: conversationId => {
                root.activePanel = "";
                root.selectConversation(conversationId);
            }
        }
    }

    Component {
        id: groupInfoComponent

        GroupInfoPanel {
            anchors.fill: parent
            groupId: root.selectedConversationId
            groupTitle: root.selectedConversationTitle
            onClosed: root.activePanel = ""
        }
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
                        color: AppTheme.textPrimary
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
