pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic
import HuskarUI.Basic

Item {
    id: root

    property string conversationTitle: qsTr("选择一个会话")
    property string conversationInitial: "?"
    property string conversationStatus: qsTr("从左侧选择联系人或群聊")
    property color conversationColor: "#7C8799"
    property bool conversationOnline: false
    property bool conversationSelected: false
    property string conversationKind: ""
    property int memberCount: 0
    property int onlineCount: 0
    property bool searchOpen: false
    readonly property string messageSearchKeyword: LanChat.messageSearchText.trim()
    readonly property color headerSurfaceColor: AppTheme.surface
    readonly property color messageBubbleColor: AppTheme.surfaceElevated

    signal cancelFileRequested(string messageId)
    signal acceptFileRequested(string messageId)
    signal openFileRequested(string filePath)
    signal revealFileRequested(string filePath)
    signal groupInfoRequested()
    signal searchFocusRequested()

    function closeSearch(): void {
        searchOpen = false;
        LanChat.messageSearchText = "";
    }

    function focusSearch(): void {
        searchOpen = true;
        Qt.callLater(() => root.searchFocusRequested());
    }

    onConversationSelectedChanged: {
        if (!conversationSelected)
            closeSearch();
    }

    Connections {
        target: LanChat.messages

        function onRowsInserted(): void {
            Qt.callLater(() => messageList.positionViewAtEnd());
        }

        function onModelReset(): void {
            Qt.callLater(() => messageList.positionViewAtEnd());
        }
    }

    Rectangle {
        id: chatHeader

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        anchors.topMargin: 10
        height: 72
        radius: AppTheme.radiusMedium
        color: root.headerSurfaceColor
        border.width: 1
        border.color: AppTheme.border

        Row {
            id: friendSummary

            anchors.left: parent.left
            anchors.right: headerActions.left
            anchors.leftMargin: 18
            anchors.rightMargin: 12
            anchors.verticalCenter: parent.verticalCenter
            spacing: 12

            HusAvatar {
                size: 44
                textSource: root.conversationInitial
                colorBg: root.conversationColor
                textSize: HusAvatar.Size_Auto

                HusBadge {
                    dot: true
                    visible: root.conversationKind === "direct"
                    badgeState: root.conversationOnline
                                ? HusBadge.State_Success
                                : HusBadge.State_Default
                }
            }

            Column {
                anchors.verticalCenter: parent.verticalCenter
                width: Math.max(0, friendSummary.width - 56)
                spacing: 5

                HusText {
                    width: parent.width
                    text: root.conversationTitle
                    color: AppTheme.textPrimary
                    elide: Text.ElideRight
                    font.pixelSize: HusTheme.Primary.fontPrimarySizeHeading5
                    font.weight: Font.Medium
                }

                HusText {
                    width: parent.width
                    text: root.conversationStatus
                    color: AppTheme.textSecondary
                    elide: Text.ElideRight
                    font.pixelSize: HusTheme.Primary.fontPrimarySize
                }
            }
        }

        Row {
            id: headerActions

            anchors.right: parent.right
            anchors.rightMargin: 14
            anchors.verticalCenter: parent.verticalCenter
            spacing: 2

            HusIconButton {
                width: 40
                height: 40
                padding: 0
                visible: root.conversationKind === "group"
                type: HusButton.Type_Text
                iconSource: HusIcon.GroupOutlined
                iconSize: 20
                contentDescription: qsTr("查看群聊信息")
                onClicked: root.groupInfoRequested()
            }

            HusIconButton {
                width: 40
                height: 40
                padding: 0
                type: root.searchOpen
                      ? HusButton.Type_Filled
                      : HusButton.Type_Text
                iconSource: HusIcon.SearchOutlined
                iconSize: 20
                enabled: root.conversationSelected
                contentDescription: root.searchOpen
                                    ? qsTr("关闭消息搜索")
                                    : qsTr("搜索当前会话")
                onClicked: root.searchOpen ? root.closeSearch() : root.focusSearch()
            }
        }
    }

    Loader {
        id: messageSearchLoader

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: chatHeader.bottom
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        anchors.topMargin: active ? 8 : 0
        height: active ? 52 : 0
        active: root.searchOpen
        sourceComponent: messageSearchComponent
    }

    Component {
        id: messageSearchComponent

        Rectangle {
            id: searchPanel

            radius: AppTheme.radiusMedium
            color: root.headerSurfaceColor
            border.width: 1
            border.color: AppTheme.border

            function focusInput(): void {
                messageSearchInput.forceActiveFocus();
                messageSearchInput.selectAll();
            }

            Component.onCompleted: searchPanel.focusInput()

            Connections {
                target: root

                function onSearchFocusRequested(): void {
                    searchPanel.focusInput();
                }
            }

            HusInput {
                id: messageSearchInput

                anchors.left: parent.left
                anchors.right: resultCount.left
                anchors.leftMargin: 14
                anchors.rightMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                height: 38
                iconSource: HusIcon.SearchOutlined
                iconPosition: HusInput.Position_Left
                clearEnabled: "active"
                type: HusInput.Type_Filled
                text: LanChat.messageSearchText
                placeholderText: qsTr("搜索消息或文件名")
                contentDescription: qsTr("搜索当前会话中的消息或文件")
                onTextChanged: LanChat.messageSearchText = text
                Keys.onEscapePressed: root.closeSearch()
            }

            HusText {
                id: resultCount

                anchors.right: parent.right
                anchors.rightMargin: 14
                anchors.verticalCenter: parent.verticalCenter
                width: 74
                text: qsTr("%1 条结果").arg(messageList.count)
                color: AppTheme.textTertiary
                horizontalAlignment: Text.AlignRight
                font.pixelSize: HusTheme.Primary.fontPrimarySize
            }
        }
    }

    ListView {
        id: messageList

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: messageSearchLoader.bottom
        anchors.bottom: parent.bottom
        anchors.leftMargin: 18
        anchors.rightMargin: 18
        anchors.topMargin: 12
        anchors.bottomMargin: 10
        model: LanChat.messages
        spacing: 10
        boundsBehavior: Flickable.StopAtBounds
        reuseItems: true
        clip: true
        ScrollBar.vertical: HusScrollBar { }

        delegate: MessageDelegate {
            id: messageDelegate

            width: messageList.width
            conversationKind: root.conversationKind
            bubbleColor: root.messageBubbleColor
            onCancelFileRequested: messageId =>
                root.cancelFileRequested(messageId)
            onAcceptFileRequested: messageId =>
                root.acceptFileRequested(messageId)
            onOpenFileRequested: filePath =>
                root.openFileRequested(filePath)
            onRevealFileRequested: filePath =>
                root.revealFileRequested(filePath)
        }
    }

    Column {
        anchors.centerIn: messageList
        width: Math.min(360, Math.max(0, messageList.width - 48))
        spacing: 10
        visible: messageList.count === 0

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            width: 64
            height: 64
            radius: 20
            color: AppTheme.accentSoft
            border.width: 1
            border.color: AppTheme.border
            Accessible.ignored: true

            HusIconText {
                anchors.centerIn: parent
                iconSource: root.conversationSelected
                            ? HusIcon.MessageOutlined
                            : HusIcon.ContactsOutlined
                iconSize: 28
                colorIcon: AppTheme.accent
            }
        }

        HusText {
            width: parent.width
            text: root.conversationSelected
                  ? root.messageSearchKeyword.length > 0
                    ? qsTr("没有找到消息")
                    : qsTr("开始聊天")
                  : qsTr("发现身边的好友")
            color: AppTheme.textPrimary
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: HusTheme.Primary.fontPrimarySizeHeading5
            font.weight: Font.Medium
        }

        HusText {
            width: parent.width
            text: root.conversationSelected
                  ? root.messageSearchKeyword.length > 0
                    ? qsTr("没有找到与“%1”匹配的消息").arg(root.messageSearchKeyword)
                    : qsTr("还没有消息，发一句问候吧")
                  : qsTr("从左侧选择联系人或群聊，即可开始聊天")
            color: AppTheme.textSecondary
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            font.pixelSize: HusTheme.Primary.fontPrimarySize
        }
    }

    Shortcut {
        sequence: StandardKey.Find
        enabled: root.conversationSelected
        onActivated: root.focusSearch()
    }

    Shortcut {
        sequence: "Escape"
        enabled: root.searchOpen
        onActivated: root.closeSearch()
    }
}
