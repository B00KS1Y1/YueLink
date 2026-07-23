pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic
import HuskarUI.Basic

Item {
    id: root

    property string friendName: qsTr("苏晚")
    property string friendInitial: qsTr("晚")
    property string friendStatus: qsTr("在线 · 局域网")
    property color friendColor: "#7C6EE6"
    property bool friendOnline: true
    property bool peerSelected: false

    signal cancelFileRequested(string messageId)
    signal openFileRequested(string filePath)
    signal revealFileRequested(string filePath)

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
        height: 78
        color: HusTheme.Primary.colorBgContainer

        Row {
            anchors.left: parent.left
            anchors.leftMargin: 24
            anchors.verticalCenter: parent.verticalCenter
            spacing: 12

            HusAvatar {
                size: 44
                textSource: root.friendInitial
                colorBg: root.friendColor
                textSize: HusAvatar.Size_Auto

                HusBadge {
                    dot: true
                    badgeState: root.friendOnline
                                ? HusBadge.State_Success
                                : HusBadge.State_Default
                }
            }

            Column {
                anchors.verticalCenter: parent.verticalCenter
                spacing: 5

                HusText {
                    text: root.friendName
                    color: HusTheme.Primary.colorTextBase
                    font.pixelSize: HusTheme.Primary.fontPrimarySizeHeading5
                    font.weight: Font.Medium
                }

                HusText {
                    text: root.friendStatus
                    color: HusTheme.Primary.colorTextSecondary
                    font.pixelSize: HusTheme.Primary.fontPrimarySize
                }
            }
        }

        Row {
            anchors.right: parent.right
            anchors.rightMargin: 18
            anchors.verticalCenter: parent.verticalCenter
            spacing: 4

            HusIconButton {
                width: 40
                height: 40
                padding: 0
                type: HusButton.Type_Text
                iconSource: HusIcon.PhoneOutlined
                iconSize: 20
                contentDescription: qsTr("语音通话")
            }

            HusIconButton {
                width: 40
                height: 40
                padding: 0
                type: HusButton.Type_Text
                iconSource: HusIcon.VideoCameraOutlined
                iconSize: 20
                contentDescription: qsTr("视频通话")
            }

            HusIconButton {
                width: 40
                height: 40
                padding: 0
                type: HusButton.Type_Text
                iconSource: HusIcon.MoreOutlined
                iconSize: 20
                contentDescription: qsTr("更多会话操作")
            }
        }
    }

    Rectangle {
        id: chatHeaderDivider

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: chatHeader.bottom
        height: 1
        color: HusTheme.Primary.colorBorderSecondary
    }

    ListView {
        id: messageList

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: chatHeaderDivider.bottom
        anchors.bottom: parent.bottom
        anchors.leftMargin: 24
        anchors.rightMargin: 18
        anchors.topMargin: 18
        anchors.bottomMargin: 12
        model: LanChat.messages
        spacing: 8
        boundsBehavior: Flickable.StopAtBounds
        reuseItems: true
        clip: true
        ScrollBar.vertical: HusScrollBar { }

        delegate: Item {
            id: messageDelegate

            required property string messageId
            required property bool fromMe
            required property string senderInitial
            required property color senderColor
            required property string messageText
            required property string messageTime
            required property string deliveryStatus
            required property string messageKind
            required property string fileName
            required property string fileSizeText
            required property real fileProgress
            required property string filePath

            readonly property bool fileTransferActive:
                deliveryStatus === "transferring"
                || deliveryStatus === "receiving"
            readonly property bool fileTransferComplete:
                deliveryStatus === "sent"
                || deliveryStatus === "received"

            width: messageList.width
            height: Math.max(messageAvatar.height, messageBody.height) + 14

            HusAvatar {
                id: messageAvatar

                anchors.left: messageDelegate.fromMe ? undefined : parent.left
                anchors.right: messageDelegate.fromMe ? parent.right : undefined
                size: 36
                textSource: messageDelegate.senderInitial
                colorBg: messageDelegate.senderColor
                textSize: HusAvatar.Size_Auto
            }

            Column {
                id: messageBody

                anchors.left: messageDelegate.fromMe ? undefined : messageAvatar.right
                anchors.right: messageDelegate.fromMe ? messageAvatar.left : undefined
                anchors.leftMargin: messageDelegate.fromMe ? 0 : 10
                anchors.rightMargin: messageDelegate.fromMe ? 10 : 0
                width: messageDelegate.messageKind === "file"
                       ? Math.min(Math.max(260, messageTextItem.implicitWidth + 28),
                                  Math.max(260, messageList.width * 0.58))
                       : Math.min(messageTextItem.implicitWidth + 28,
                                  Math.max(220, messageList.width * 0.58))
                spacing: 5

                Rectangle {
                    width: parent.width
                    height: messageTextItem.implicitHeight
                            + (messageDelegate.messageKind === "file" ? 36 : 20)
                    radius: 10
                    color: messageDelegate.fromMe
                           ? HusTheme.Primary.colorPrimaryBg
                           : HusTheme.Primary.colorBgContainer
                    border.width: messageDelegate.fromMe ? 0 : 1
                    border.color: HusTheme.Primary.colorBorderSecondary

                    HusText {
                        id: messageTextItem

                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.bottom: messageDelegate.messageKind === "file"
                                        ? fileProgressTrack.top
                                        : parent.bottom
                        anchors.leftMargin: 14
                        anchors.rightMargin: 14
                        anchors.topMargin: 10
                        anchors.bottomMargin: messageDelegate.messageKind === "file" ? 8 : 10
                        text: messageDelegate.messageKind === "file"
                              ? qsTr("文件：%1\n%2").arg(messageDelegate.fileName)
                                                       .arg(messageDelegate.fileSizeText)
                              : messageDelegate.messageText
                        color: HusTheme.Primary.colorTextBase
                        wrapMode: Text.Wrap
                        font.pixelSize: HusTheme.Primary.fontPrimarySize
                    }

                    Rectangle {
                        id: fileProgressTrack

                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        anchors.leftMargin: 14
                        anchors.rightMargin: 14
                        anchors.bottomMargin: 10
                        height: 4
                        visible: messageDelegate.messageKind === "file"
                        radius: 2
                        color: HusTheme.Primary.colorFillSecondary

                        Rectangle {
                            anchors.left: parent.left
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            width: parent.width * Math.max(0, Math.min(1,
                                                                       messageDelegate.fileProgress))
                            radius: parent.radius
                            color: messageDelegate.deliveryStatus === "failed"
                                   ? "#D84A4A"
                                   : messageDelegate.deliveryStatus === "cancelled"
                                     ? HusTheme.Primary.colorTextTertiary
                                     : HusTheme.Primary.colorPrimary
                        }
                    }
                }

                HusText {
                    width: parent.width
                    text: messageDelegate.deliveryStatus === "failed"
                          ? (messageDelegate.fromMe
                             ? qsTr("%1 · 发送失败").arg(messageDelegate.messageTime)
                             : qsTr("%1 · 接收失败").arg(messageDelegate.messageTime))
                          : messageDelegate.deliveryStatus === "cancelled"
                            ? qsTr("%1 · 已取消").arg(messageDelegate.messageTime)
                          : messageDelegate.deliveryStatus === "transferring"
                            ? qsTr("%1 · 正在传输 %2%")
                                  .arg(messageDelegate.messageTime)
                                  .arg(Math.round(messageDelegate.fileProgress * 100))
                            : messageDelegate.deliveryStatus === "receiving"
                              ? qsTr("%1 · 正在接收 %2%")
                                    .arg(messageDelegate.messageTime)
                                    .arg(Math.round(messageDelegate.fileProgress * 100))
                              : messageDelegate.messageKind === "file"
                                && messageDelegate.deliveryStatus === "sent"
                                ? qsTr("%1 · 已发送").arg(messageDelegate.messageTime)
                                : messageDelegate.messageKind === "file"
                                  && messageDelegate.deliveryStatus === "received"
                                  ? qsTr("%1 · 已保存到下载/YueLink")
                                        .arg(messageDelegate.messageTime)
                          : messageDelegate.deliveryStatus === "sending"
                            ? qsTr("%1 · 发送中").arg(messageDelegate.messageTime)
                            : messageDelegate.messageTime
                    color: messageDelegate.deliveryStatus === "failed"
                           ? "#D84A4A"
                           : HusTheme.Primary.colorTextTertiary
                    horizontalAlignment: messageDelegate.fromMe
                                         ? Text.AlignRight
                                         : Text.AlignLeft
                    font.pixelSize: HusTheme.Primary.fontPrimarySize
                }

                Row {
                    id: fileActionRow

                    x: messageDelegate.fromMe ? parent.width - width : 0
                    height: visible ? implicitHeight : 0
                    spacing: 4
                    visible: messageDelegate.messageKind === "file"
                             && (messageDelegate.fileTransferActive
                                 || (messageDelegate.fileTransferComplete
                                     && messageDelegate.filePath.length > 0))

                    HusButton {
                        height: 30
                        visible: messageDelegate.fileTransferActive
                        type: HusButton.Type_Text
                        text: qsTr("取消")
                        contentDescription: qsTr("取消文件传输")
                        onClicked: root.cancelFileRequested(messageDelegate.messageId)
                    }

                    HusButton {
                        height: 30
                        visible: messageDelegate.fileTransferComplete
                                 && messageDelegate.filePath.length > 0
                        type: HusButton.Type_Text
                        text: qsTr("打开")
                        contentDescription: qsTr("打开文件")
                        onClicked: root.openFileRequested(messageDelegate.filePath)
                    }

                    HusButton {
                        height: 30
                        visible: messageDelegate.fileTransferComplete
                                 && messageDelegate.filePath.length > 0
                        type: HusButton.Type_Text
                        text: qsTr("所在位置")
                        contentDescription: qsTr("在文件夹中显示")
                        onClicked: root.revealFileRequested(messageDelegate.filePath)
                    }
                }
            }
        }
    }

    HusText {
        anchors.centerIn: messageList
        visible: messageList.count === 0
        text: root.peerSelected
              ? qsTr("还没有消息，发一句问候吧")
              : qsTr("从左侧选择一个局域网好友开始聊天")
        color: HusTheme.Primary.colorTextTertiary
        font.pixelSize: HusTheme.Primary.fontPrimarySize
    }
}
