pragma ComponentBehavior: Bound

import QtQuick
import HuskarUI.Basic

Item {
    id: root

    required property string messageId
    required property bool fromMe
    required property string senderName
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
    required property url fileUrl
    required property int imageWidth
    required property int imageHeight
    required property string emojiPackageId
    required property string emojiId
    required property int deliveredCount
    required property int totalRecipientCount

    property string conversationKind: ""
    property color bubbleColor: "transparent"
    readonly property bool transferActive: deliveryStatus === "transferring"
                                           || deliveryStatus === "receiving"
    readonly property bool awaitingAcceptance:
        deliveryStatus === "awaiting_acceptance"
    readonly property bool transferComplete: deliveryStatus === "sent"
                                             || deliveryStatus === "received"
    readonly property bool attachmentMessage: messageKind === "image"
                                              || messageKind === "file"
    readonly property bool imagePreviewAvailable:
        messageKind === "image" && fileUrl.toString().length > 0
        && (fromMe || transferComplete)

    signal cancelFileRequested(string messageId)
    signal acceptFileRequested(string messageId)
    signal openFileRequested(string filePath)
    signal revealFileRequested(string filePath)

    function deliverySummary(): string {
        if (deliveryStatus === "failed")
            return fromMe ? qsTr("%1 · 发送失败").arg(messageTime)
                          : qsTr("%1 · 接收失败").arg(messageTime);
        if (deliveryStatus === "cancelled")
            return qsTr("%1 · 已取消").arg(messageTime);
        if (deliveryStatus === "awaiting_acceptance")
            return fromMe ? qsTr("%1 · 等待对方接收").arg(messageTime)
                          : qsTr("%1 · 等待接收").arg(messageTime);
        if (deliveryStatus === "transferring")
            return qsTr("%1 · 正在传输 %2%").arg(messageTime)
                    .arg(Math.round(fileProgress * 100));
        if (deliveryStatus === "receiving")
            return qsTr("%1 · 正在接收 %2%").arg(messageTime)
                    .arg(Math.round(fileProgress * 100));
        if (fromMe && totalRecipientCount > 0)
            return qsTr("%1 · 已发送 %2/%3").arg(messageTime)
                    .arg(deliveredCount).arg(totalRecipientCount);
        if (deliveryStatus === "sending" || deliveryStatus === "pending")
            return qsTr("%1 · 发送中").arg(messageTime);
        return messageTime;
    }

    ListView.onPooled: contentLoader.active = false
    ListView.onReused: contentLoader.active = true

    implicitHeight: Math.max(messageAvatar.height, messageBody.height) + 14
    height: implicitHeight

    HusAvatar {
        id: messageAvatar

        x: root.fromMe ? root.width - width : 0
        width: 36
        height: 36
        size: 36
        textSource: root.senderInitial
        colorBg: root.senderColor
        textSize: HusAvatar.Size_Auto
    }

    Column {
        id: messageBody

        x: root.fromMe ? messageAvatar.x - width - 10
                       : messageAvatar.x + messageAvatar.width + 10
        width: Math.min(root.messageKind === "image" ? 360
                        : root.messageKind === "file" ? 288
                        : root.messageKind === "emoji" ? 84 : 420,
                        Math.max(root.messageKind === "emoji" ? 84 : 220,
                                 root.width * 0.58))
        spacing: 5

        HusText {
            width: parent.width
            visible: root.conversationKind === "group" && !root.fromMe
            text: root.senderName
            color: HusTheme.Primary.colorTextSecondary
            elide: Text.ElideRight
            font.pixelSize: Math.max(11, HusTheme.Primary.fontPrimarySize - 1)
            font.weight: Font.Medium
        }

        Rectangle {
            width: parent.width
            implicitHeight: contentLoader.implicitHeight + 20
            radius: 14
            color: root.fromMe ? HusTheme.Primary.colorPrimaryBg : root.bubbleColor
            border.width: root.fromMe ? 0 : 1
            border.color: HusTheme.Primary.colorBorderSecondary

            Loader {
                id: contentLoader

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 10
                active: true
                sourceComponent: root.messageKind === "image" ? imageComponent
                                 : root.messageKind === "file" ? fileComponent
                                 : root.messageKind === "emoji" ? emojiComponent
                                 : textComponent
            }
        }

        HusText {
            width: parent.width
            text: root.deliverySummary()
            color: root.deliveryStatus === "failed"
                   ? "#D84A4A"
                   : HusTheme.Primary.colorTextTertiary
            horizontalAlignment: root.fromMe ? Text.AlignRight : Text.AlignLeft
            font.pixelSize: HusTheme.Primary.fontPrimarySize
        }

        Row {
            x: root.fromMe ? parent.width - width : 0
            height: visible ? implicitHeight : 0
            spacing: 4
            visible: root.attachmentMessage
                     && (root.awaitingAcceptance
                         || root.transferActive
                         || (root.transferComplete && root.filePath.length > 0))

            HusButton {
                height: 30
                visible: root.awaitingAcceptance && !root.fromMe
                type: HusButton.Type_Primary
                text: qsTr("接收")
                onClicked: root.acceptFileRequested(root.messageId)
            }

            HusButton {
                height: 30
                visible: root.transferActive
                         || (root.awaitingAcceptance && root.fromMe)
                type: HusButton.Type_Text
                text: qsTr("取消")
                onClicked: root.cancelFileRequested(root.messageId)
            }

            HusButton {
                height: 30
                visible: root.transferComplete && root.filePath.length > 0
                type: HusButton.Type_Text
                text: qsTr("打开")
                onClicked: root.openFileRequested(root.filePath)
            }

            HusButton {
                height: 30
                visible: root.transferComplete && root.filePath.length > 0
                type: HusButton.Type_Text
                text: qsTr("所在位置")
                onClicked: root.revealFileRequested(root.filePath)
            }
        }
    }

    Component {
        id: textComponent

        TextMessageContent { messageText: root.messageText }
    }

    Component {
        id: imageComponent

        ImageMessageContent {
            fileUrl: root.fileUrl
            fileName: root.fileName
            caption: root.messageText
            previewAvailable: root.imagePreviewAvailable
            progress: root.fileProgress
            deliveryStatus: root.deliveryStatus
        }
    }

    Component {
        id: fileComponent

        FileMessageContent {
            fileName: root.fileName
            fileSizeText: root.fileSizeText
            progress: root.fileProgress
            deliveryStatus: root.deliveryStatus
        }
    }

    Component {
        id: emojiComponent

        EmojiMessageContent {
            emojiPackageId: root.emojiPackageId
            emojiId: root.emojiId
            fallbackText: root.messageText
        }
    }
}
