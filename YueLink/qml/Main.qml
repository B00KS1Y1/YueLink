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
    property bool chatHistoryOpen: false
    readonly property color shellSurfaceColor: HusTheme.Primary.colorBgBase
    readonly property color shellDividerColor: HusTheme.Primary.colorSplit
    readonly property url windowBackgroundSource: AppSettings.theme.backgroundImage
    readonly property color translucentSurfaceColor: HusThemeFunctions.alpha(
                                                         root.shellSurfaceColor,
                                                         AppSettings.theme.backgroundOpacity)
    readonly property color controlSurfaceColor: HusThemeFunctions.alpha(
                                                     root.shellSurfaceColor,
                                                     AppSettings.theme.backgroundOpacity * 0.55)
    readonly property color navigationSelectedIconColor: HusTheme.isDark
                                                          ? HusTheme.Primary.colorPrimary
                                                          : HusTheme.Primary.colorTextBase
    readonly property int navigationWidth: 60
    readonly property int conversationListWidth: Math.round(
                                                      Math.min(320,
                                                               Math.max(280,
                                                                        width * 0.26)))

    captionBar.height: 42
    captionBar.color: root.translucentSurfaceColor
    captionBar.showWinIcon: false
    captionBar.winTitle: qsTr("YueLink")
    captionBar.winTitleDelegate: Item {
        id: captionTitleContent

        implicitWidth: captionTitleRow.implicitWidth
        implicitHeight: root.captionBar.height
        width: implicitWidth
        height: root.captionBar.height

        Row {
            id: captionTitleRow

            anchors.verticalCenter: parent.verticalCenter
            height: parent.height
            spacing: 10

            Item {
                id: appBrand

                width: appBrandRow.implicitWidth + 12
                height: parent.height

                Row {
                    id: appBrandRow

                    anchors.centerIn: parent
                    spacing: 7

                    Image {
                        anchors.verticalCenter: parent.verticalCenter
                        width: 22
                        height: 22
                        source: "qrc:/yuelink/assets/yuelink-app-icon.png"
                        sourceSize.width: 22
                        sourceSize.height: 22
                        fillMode: Image.PreserveAspectFit
                        Accessible.ignored: true
                    }

                    HusText {
                        anchors.verticalCenter: parent.verticalCenter
                        text: qsTr("YueLink")
                        color: HusTheme.Primary.colorTextBase
                        font.family: HusTheme.Primary.fontPrimaryFamily
                        font.pixelSize: Math.max(15, HusTheme.Primary.fontPrimarySize)
                        font.weight: Font.DemiBold
                        font.letterSpacing: 0.5
                    }
                }
            }

            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                width: 1
                height: 18
                color: root.shellDividerColor
                Accessible.ignored: true
            }

            UserInfoPanel {
                id: headerProfile
                width: 176
                height: root.captionBar.height
                enabled: root.activePanel !== "settings"
                onProfileEditRequested: root.openPanel("profile")
            }
        }

        Connections {
            target: root.captionBar
            function onWindowAgentChanged(): void {
                root.captionBar.addInteractionItem(headerProfile)
            }
        }

        Component.onDestruction: {
            root.captionBar.removeInteractionItem(headerProfile);
        }
    }
    captionBar.winExtraButtonsDelegate: Row {
        id: captionExtraButtons

        height: parent.height

        Component.onCompleted: root.captionBar.addInteractionItem(settingsCaptionButton)
        Component.onDestruction: root.captionBar.removeInteractionItem(settingsCaptionButton)

        Connections {
            target: root.captionBar

            function onWindowAgentChanged(): void {
                root.captionBar.addInteractionItem(settingsCaptionButton)
            }
        }

        HusCaptionButton {
            id: settingsCaptionButton
            height: parent.height
            enabled: root.activePanel !== "settings"
            noDisabledState: true
            iconSource: HusIcon.SettingOutlined
            iconSize: 16
            contentDescription: qsTr("设置")
            onClicked: root.openPanel("settings")
        }
    }

    function applyAppearanceSettings(): void {
        HusTheme.darkMode = AppSettings.theme.mode === "light"
                ? HusTheme.Light
                : AppSettings.theme.mode === "system"
                  ? HusTheme.System
                  : HusTheme.Dark;
        HusTheme.installThemePrimaryColorBase(AppSettings.theme.primaryColor);
        HusTheme.animationEnabled = AppSettings.theme.animationsEnabled;
    }

    function openPanel(panel: string): void {
        activePanel = panel;
    }

    function openChatHistory(): void {
        if (selectedConversationId.length === 0)
            return;
        if (!chatHistoryOpen) {
            chatHistoryOpen = true;
            return;
        }
        if (chatHistoryLoader.status === Loader.Ready) {
            chatHistoryLoader.item.raise();
            chatHistoryLoader.item.requestActivate();
            chatHistoryLoader.item.focusSearch();
        }
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
        if (conversation.itemId === undefined || conversation.hidden === true) {
            selectedConversationId = "";
            refreshSelectedConversation();
            return;
        }

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
        LanChat.setNotificationsEnabled(AppSettings.application.notificationsEnabled);
    }

    Connections {
        target: AppSettings.theme

        function onSettingsChanged(): void {
            root.applyAppearanceSettings();
        }
    }

    Connections {
        target: AppSettings.application

        function onNotificationsEnabledChanged(): void {
            LanChat.setNotificationsEnabled(AppSettings.application.notificationsEnabled);
        }
    }

    Connections {
        target: AppSettings.log

        function onRestartRequiredChanged(): void {
            if (AppSettings.log.restartRequired)
                settingsNotification.warning(qsTr("需要重新启动"),
                                             qsTr("设置已保存，将在重新启动应用后生效"),
                                             5000);
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

        function onConversationRemoved(conversationId: string): void {
            if (conversationId === root.selectedConversationId) {
                root.chatHistoryOpen = false;
                root.selectedConversationId = "";
                root.activePanel = "";
                root.refreshSelectedConversation();
            }
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
    color: root.shellSurfaceColor

    onActiveChanged: {
        if (active && selectedConversationId.length > 0)
            LanChat.markConversationRead(selectedConversationId);
    }

    Rectangle {
        id: applicationBackground

        anchors.fill: parent
        color: root.shellSurfaceColor
        Accessible.ignored: true

        Image {
            id: windowBackgroundImage

            anchors.fill: parent
            source: root.windowBackgroundSource
            sourceSize.width: 1586
            sourceSize.height: 992
            asynchronous: true
            cache: true
            fillMode: Image.Stretch
            smooth: true
            visible: status === Image.Ready
            Accessible.ignored: true
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: root.captionBar.height - 1
        height: 1
        color: root.shellDividerColor
        Accessible.ignored: true
    }

    Item {
        id: mainWorkspace

        anchors.fill: parent
        anchors.topMargin: root.captionBar.height
        visible: root.activePanel !== "settings"

        Rectangle {
            id: navigationRail

            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: root.navigationWidth
            radius: 0
            color: root.translucentSurfaceColor
            border.width: 0

            Column {
                id: primaryNavigationButtons

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 8
                spacing: 6

                HusIconButton {
                    id: messagesNavigationButton

                    width: primaryNavigationButtons.width
                    height: 44
                    text: ""
                    iconSource: HusIcon.MessageOutlined
                    iconSize: 20
                    type: HusButton.Type_Default
                    padding: 0
                    colorBg: root.currentPage === "messages"
                             ? HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, HusTheme.isDark ? 0.28 : 0.16)
                             : hovered ? HusTheme.Primary.colorFillSecondary : "transparent"
                    colorIcon: root.currentPage === "messages"
                               ? root.navigationSelectedIconColor
                               : HusTheme.Primary.colorTextTertiary
                    colorBorder: visualFocus ? HusTheme.Primary.colorPrimary : "transparent"
                    borderWidth: visualFocus ? 1 : 0
                    radiusBg.all: 10
                    contentDescription: qsTr("消息")
                    onClicked: root.currentPage = "messages"

                    Rectangle {
                        anchors.left: parent.left
                        anchors.leftMargin: 4
                        anchors.verticalCenter: parent.verticalCenter
                        width: 3
                        height: 16
                        radius: 2
                        visible: root.currentPage === "messages"
                        color: HusTheme.Primary.colorPrimary
                        Accessible.ignored: true
                    }

                    HusToolTip {
                        visible: messagesNavigationButton.hovered || messagesNavigationButton.visualFocus
                        showArrow: true
                        position: HusToolTip.Position_Right
                        text: messagesNavigationButton.contentDescription
                    }
                }

                HusIconButton {
                    id: contactsNavigationButton

                    width: primaryNavigationButtons.width
                    height: 44
                    text: ""
                    iconSource: HusIcon.ContactsOutlined
                    iconSize: 20
                    type: HusButton.Type_Default
                    padding: 0
                    colorBg: root.currentPage === "contacts"
                             ? HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, HusTheme.isDark ? 0.28 : 0.16)
                             : hovered ? HusTheme.Primary.colorFillSecondary : "transparent"
                    colorIcon: root.currentPage === "contacts"
                               ? root.navigationSelectedIconColor
                               : HusTheme.Primary.colorTextTertiary
                    colorBorder: visualFocus ? HusTheme.Primary.colorPrimary : "transparent"
                    borderWidth: visualFocus ? 1 : 0
                    radiusBg.all: 10
                    contentDescription: qsTr("联系人")
                    onClicked: root.currentPage = "contacts"

                    Rectangle {
                        anchors.left: parent.left
                        anchors.leftMargin: 4
                        anchors.verticalCenter: parent.verticalCenter
                        width: 3
                        height: 16
                        radius: 2
                        visible: root.currentPage === "contacts"
                        color: HusTheme.Primary.colorPrimary
                        Accessible.ignored: true
                    }

                    HusToolTip {
                        visible: contactsNavigationButton.hovered || contactsNavigationButton.visualFocus
                        showArrow: true
                        position: HusToolTip.Position_Right
                        text: contactsNavigationButton.contentDescription
                    }
                }
            }

            HusIconButton {
                id: refreshPeersButton

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 8
                height: 44
                text: ""
                iconSource: HusIcon.ReloadOutlined
                iconSize: 20
                type: HusButton.Type_Default
                padding: 0
                colorBg: hovered ? HusTheme.Primary.colorFillSecondary : "transparent"
                colorIcon: HusTheme.Primary.colorTextTertiary
                colorBorder: visualFocus ? HusTheme.Primary.colorPrimary : "transparent"
                borderWidth: visualFocus ? 1 : 0
                radiusBg.all: 10
                contentDescription: qsTr("刷新好友")
                onClicked: {
                    if (LanChat.running)
                        root.refreshNetworkDiscovery();
                    else
                        root.startNetworkService();
                }

                HusToolTip {
                    visible: refreshPeersButton.hovered || refreshPeersButton.visualFocus
                    showArrow: true
                    position: HusToolTip.Position_Right
                    text: refreshPeersButton.contentDescription
                }
            }

            Rectangle {
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 1
                color: root.shellDividerColor
                Accessible.ignored: true
            }
        }

        Rectangle {
            id: friendPanel

            anchors.left: navigationRail.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: root.conversationListWidth
            radius: 0
            color: root.translucentSurfaceColor
            border.width: 0

            FriendListPanel {
                id: friendList

                anchors.fill: parent
                selectedConversationId: root.selectedConversationId
                contactsMode: root.currentPage === "contacts"
                searchSurfaceColor: "transparent"
                controlSurfaceColor: root.controlSurfaceColor
                onConversationSelected: conversationId =>
                                        root.selectConversation(conversationId)
                onCreateGroupRequested: {
                    LanChat.peerSearchText = "";
                    root.openPanel("createGroup");
                }
            }

            Rectangle {
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 1
                color: root.shellDividerColor
                Accessible.ignored: true
            }
        }

        Rectangle {
            id: conversationPanel

            anchors.left: friendPanel.right
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            radius: 0
            color: root.translucentSurfaceColor
            border.width: 0

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
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                anchors.bottomMargin: 10
                height: implicitHeight
                conversationId: root.selectedConversationId
                sendEnabled: root.selectedConversationId.length > 0
                             && (root.selectedConversationKind === "group"
                                 || root.selectedConversationOnline)
                filesEnabled: root.selectedConversationKind === "direct"
                              && root.selectedConversationOnline
                inputSurfaceColor: root.controlSurfaceColor
                onImagesSelected: imageUrls => {
                    if (root.selectedConversationId.length > 0)
                        LanChat.sendImages(root.selectedConversationId, imageUrls);
                }
                onFilesSelected: fileUrls => {
                    if (root.selectedConversationId.length > 0)
                        LanChat.sendFiles(root.selectedConversationId, fileUrls);
                }
                onHistoryRequested: root.openChatHistory()
            }
        }
    }

    HusMessage {
        id: operationMessage

        anchors.fill: parent
        z: 1000
    }

    HusNotification {
        id: settingsNotification

        anchors.fill: parent
        position: HusNotification.Position_TopRight
        topMargin: root.captionBar.height + 12
        z: 1000
    }

    Loader {
        id: chatHistoryLoader

        active: root.chatHistoryOpen
        sourceComponent: chatHistoryComponent
        onLoaded: {
            item.raise();
            item.requestActivate();
        }
    }

    Loader {
        active: root.activePanel === "profile"
        sourceComponent: profilePanelComponent
    }

    Loader {
        anchors.fill: parent
        anchors.topMargin: root.captionBar.height
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
        id: chatHistoryComponent

        ChatHistoryWindow {
            ownerWindow: root
            conversationTitle: root.selectedConversationTitle
            onCloseRequested: root.chatHistoryOpen = false
        }
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

        SettingsPage {
            anchors.fill: parent
            navigationSurfaceColor: root.translucentSurfaceColor
            contentSurfaceColor: root.translucentSurfaceColor
            controlSurfaceColor: root.controlSurfaceColor
            onCloseRequested: root.activePanel = ""
        }
    }
}
