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

    signal cancelFileRequested(string messageId)
    signal acceptFileRequested(string messageId)
    signal openFileRequested(string filePath)
    signal revealFileRequested(string filePath)
    signal groupInfoRequested()

    Connections {
        target: LanChat.messages

        function onRowsInserted(): void {
            Qt.callLater(() => messageList.positionViewAtEnd());
        }

        function onModelReset(): void {
            Qt.callLater(() => messageList.positionViewAtEnd());
        }
    }

    Item {
        id: chatHeader

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 48

        Row {
            id: friendSummary

            anchors.left: parent.left
            anchors.right: headerActions.left
            anchors.leftMargin: 18
            anchors.rightMargin: 12
            anchors.verticalCenter: parent.verticalCenter
            spacing: 8

            HusText {
                id: conversationTitleLabel

                width: Math.min(implicitWidth,
                                Math.max(0, friendSummary.width
                                         - (conversationPresence.visible
                                            ? conversationPresence.width + friendSummary.spacing
                                            : 0)))
                text: root.conversationTitle
                color: HusTheme.Primary.colorTextBase
                elide: Text.ElideRight
                font.pixelSize: HusTheme.Primary.fontPrimarySizeHeading5
                font.weight: Font.Medium
            }

            HusBadge {
                id: conversationPresence

                anchors.verticalCenter: parent.verticalCenter
                dot: true
                visible: root.conversationKind === "direct"
                badgeState: root.conversationOnline
                            ? HusBadge.State_Success
                            : HusBadge.State_Default
                Accessible.name: root.conversationOnline
                                 ? qsTr("在线")
                                 : qsTr("离线")
            }
        }

        Row {
            id: headerActions

            anchors.right: parent.right
            anchors.rightMargin: 14
            anchors.verticalCenter: parent.verticalCenter
            spacing: 2

            HusIconButton {
                width: 36
                height: 36
                padding: 0
                visible: root.conversationKind === "group"
                type: HusButton.Type_Text
                iconSource: HusIcon.GroupOutlined
                iconSize: 18
                contentDescription: qsTr("查看群聊信息")
                onClicked: root.groupInfoRequested()
            }

        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 1
            color: HusTheme.Primary.colorSplit
            Accessible.ignored: true
        }

    }

    ListView {
        id: messageList

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: chatHeader.bottom
        anchors.bottom: parent.bottom
        anchors.leftMargin: 22
        anchors.rightMargin: 22
        anchors.topMargin: 10
        anchors.bottomMargin: 12
        model: LanChat.messages
        spacing: 6
        boundsBehavior: Flickable.StopAtBounds
        reuseItems: true
        clip: true
        ScrollBar.vertical: HusScrollBar {
            contentDescription: qsTr("消息列表滚动条")
        }

        delegate: MessageDelegate {
            id: messageDelegate

            width: messageList.width
            conversationKind: root.conversationKind
            outgoingRightInset: 18
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
            color: HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, HusTheme.isDark ? 0.18 : 0.1)
            border.width: 1
            border.color: HusTheme.Primary.colorSplit
            Accessible.ignored: true

            HusIconText {
                anchors.centerIn: parent
                iconSource: root.conversationSelected
                            ? HusIcon.MessageOutlined
                            : HusIcon.ContactsOutlined
                iconSize: 28
                colorIcon: HusTheme.Primary.colorPrimary
            }
        }

        HusText {
            width: parent.width
            text: root.conversationSelected
                  ? qsTr("开始聊天")
                  : qsTr("发现身边的好友")
            color: HusTheme.Primary.colorTextBase
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: HusTheme.Primary.fontPrimarySizeHeading5
            font.weight: Font.Medium
        }

        HusText {
            width: parent.width
            text: root.conversationSelected
                  ? qsTr("还没有消息，发一句问候吧")
                  : qsTr("从左侧选择联系人或群聊，即可开始聊天")
            color: HusTheme.Primary.colorTextTertiary
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            font.pixelSize: HusTheme.Primary.fontPrimarySize
        }
    }

}
