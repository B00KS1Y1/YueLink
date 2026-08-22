pragma ComponentBehavior: Bound

import QtQuick
import HuskarUI.Basic

Item {
    id: root

    required property int index
    required property string messageId
    required property bool fromMe
    required property string senderName
    required property string senderInitial
    required property color senderColor
    required property string messageText
    required property string messageTime
    required property bool showTime
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
    readonly property int metadataFontSize:
        Math.max(12, HusTheme.Primary.fontPrimarySize - 2)
    readonly property string deliveryText: deliverySummary()
    readonly property bool timeVisible:
        messageTime.length > 0 && (showTime || index === 0)
    readonly property bool deliveryStatusVisible: deliveryText.length > 0
    readonly property real contentTopMargin: timeVisible ? 30 : 0
    readonly property real availableBubbleWidth:
        Math.max(0, width - avatarHalo.width - 12)
    readonly property real maximumTextBubbleWidth:
        Math.min(420, availableBubbleWidth, width * 0.58)
    readonly property real preferredBubbleWidth: {
        if (messageKind === "image")
            return Math.min(360, availableBubbleWidth);
        if (messageKind === "file")
            return Math.min(288, availableBubbleWidth);
        if (messageKind === "emoji")
            return Math.min(88, availableBubbleWidth);

        const measuredWidth = Math.ceil(messageTextMetrics.advanceWidth) + 28;
        return Math.min(maximumTextBubbleWidth,
                        Math.max(48, measuredWidth));
    }
    readonly property real preferredStatusWidth:
        deliveryStatusVisible
        ? Math.min(availableBubbleWidth,
                   Math.max(48, Math.ceil(deliveryTextMetrics.advanceWidth) + 16))
        : 0
    readonly property real preferredSenderWidth:
        conversationKind === "group" && !fromMe
        ? Math.min(availableBubbleWidth,
                   Math.ceil(senderNameMetrics.advanceWidth) + 4)
        : 0
    readonly property real preferredBodyWidth:
        Math.max(preferredBubbleWidth,
                 preferredStatusWidth,
                 preferredSenderWidth)

    signal cancelFileRequested(string messageId)
    signal acceptFileRequested(string messageId)
    signal openFileRequested(string filePath)
    signal revealFileRequested(string filePath)

    function deliverySummary(): string {
        if (deliveryStatus === "failed")
            return fromMe ? qsTr("发送失败") : qsTr("接收失败");
        if (deliveryStatus === "cancelled")
            return qsTr("已取消");
        if (deliveryStatus === "awaiting_acceptance")
            return fromMe ? qsTr("等待对方接收") : qsTr("等待接收");
        if (deliveryStatus === "transferring")
            return qsTr("正在传输 %1%").arg(Math.round(fileProgress * 100));
        if (deliveryStatus === "receiving")
            return qsTr("正在接收 %1%").arg(Math.round(fileProgress * 100));
        if (fromMe && totalRecipientCount > 0)
            return qsTr("已发送 %1/%2").arg(deliveredCount)
                    .arg(totalRecipientCount);
        if (deliveryStatus === "sending" || deliveryStatus === "pending")
            return qsTr("发送中");
        return "";
    }

    ListView.onPooled: contentLoader.active = false
    ListView.onReused: contentLoader.active = true

    TextMetrics {
        id: messageTextMetrics

        font.family: HusTheme.HusCopyableText.fontFamily
        font.pixelSize: HusTheme.Primary.fontPrimarySize
        text: root.messageText
    }

    TextMetrics {
        id: deliveryTextMetrics

        font.family: HusTheme.Primary.fontPrimaryFamily
        font.pixelSize: root.metadataFontSize
        text: root.deliveryText
    }

    TextMetrics {
        id: senderNameMetrics

        font.family: HusTheme.Primary.fontPrimaryFamily
        font.pixelSize: Math.max(11, HusTheme.Primary.fontPrimarySize - 1)
        font.weight: Font.Medium
        text: root.senderName
    }

    TextMetrics {
        id: timeTextMetrics

        font.family: HusTheme.Primary.fontPrimaryFamily
        font.pixelSize: root.metadataFontSize
        text: root.messageTime
    }

    implicitHeight: root.contentTopMargin
                    + Math.max(avatarHalo.height, messageBody.height) + 10
    height: implicitHeight

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.max(56, Math.ceil(timeTextMetrics.advanceWidth) + 20)
        height: root.timeVisible ? 22 : 0
        visible: root.timeVisible
        radius: height / 2
        color: HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary,
                                       HusTheme.isDark ? 0.13 : 0.07)
        border.width: 1
        border.color: HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary,
                                              HusTheme.isDark ? 0.28 : 0.16)

        HusText {
            anchors.centerIn: parent
            text: root.messageTime
            color: HusTheme.Primary.colorTextTertiary
            font.pixelSize: root.metadataFontSize
        }
    }

    Rectangle {
        id: avatarHalo

        y: root.contentTopMargin
        x: root.fromMe ? root.width - width : 0
        width: 40
        height: 40
        radius: width / 2
        color: HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary,
                                       HusTheme.isDark ? 0.16 : 0.1)
        border.width: 1
        border.color: HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary,
                                              HusTheme.isDark ? 0.42 : 0.28)
        Accessible.ignored: true

        HusAvatar {
            anchors.centerIn: parent
            size: 34
            textSource: root.senderInitial
            colorBg: root.senderColor
            textSize: HusAvatar.Size_Auto
        }
    }

    Column {
        id: messageBody

        y: root.contentTopMargin
        x: root.fromMe ? avatarHalo.x - width - 12
                       : avatarHalo.x + avatarHalo.width + 12
        width: root.preferredBodyWidth
        spacing: 4

        HusText {
            x: 2
            width: parent.width
            visible: root.conversationKind === "group" && !root.fromMe
            text: root.senderName
            color: HusTheme.Primary.colorPrimary
            elide: Text.ElideRight
            font.pixelSize: Math.max(11, HusTheme.Primary.fontPrimarySize - 1)
            font.weight: Font.Medium
        }

        Item {
            id: messageBubbleFrame

            x: root.fromMe ? parent.width - width : 0
            width: root.preferredBubbleWidth
            implicitHeight: contentLoader.implicitHeight + 20

            Rectangle {
                y: 2
                width: parent.width
                height: parent.height
                radius: HusTheme.Primary.radiusPrimaryLG
                color: HusThemeFunctions.alpha(HusTheme.HusCard.colorShadow,
                                               HusTheme.isDark ? 0.1 : 0.08)
                Accessible.ignored: true
            }

            Rectangle {
                id: messageBubble

                anchors.fill: parent
                radius: HusTheme.Primary.radiusPrimaryLG
                color: HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary,
                                               HusTheme.isDark ? 0.28 : 0.16)
                border.width: 1
                border.color: HusThemeFunctions.alpha(
                                  HusTheme.Primary.colorPrimary,
                                  HusTheme.isDark ? 0.54 : 0.32)

                Loader {
                    id: contentLoader

                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12
                    anchors.topMargin: 10
                    active: true
                    sourceComponent: root.messageKind === "image" ? imageComponent
                                     : root.messageKind === "file" ? fileComponent
                                     : root.messageKind === "emoji" ? emojiComponent
                                     : textComponent
                }
            }
        }

        Rectangle {
            id: deliveryStatusPill

            x: root.fromMe ? parent.width - width : 0
            width: root.preferredStatusWidth
            height: visible ? 22 : 0
            visible: root.deliveryStatusVisible
            radius: height / 2
            color: HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary,
                                           HusTheme.isDark ? 0.13 : 0.07)
            border.width: 1
            border.color: HusThemeFunctions.alpha(
                              root.deliveryStatus === "failed"
                              ? HusTheme.Primary.colorError
                              : HusTheme.Primary.colorPrimary,
                              HusTheme.isDark ? 0.28 : 0.16)

            HusText {
                anchors.centerIn: parent
                text: root.deliveryText
                color: root.deliveryStatus === "failed"
                       ? HusTheme.Primary.colorError
                       : HusTheme.Primary.colorTextTertiary
                font.pixelSize: root.metadataFontSize
            }
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
