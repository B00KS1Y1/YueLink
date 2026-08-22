pragma ComponentBehavior: Bound

import QtQuick
import HuskarUI.Basic

Item {
    id: root

    required property string senderName
    required property string senderInitial
    required property color senderColor
    required property string messageText
    required property string messageTime
    required property string messageKind
    required property string fileName
    required property string fileSizeText

    readonly property var kindIcon: messageKind === "shake"
                                    ? HusIcon.ShakeOutlined
                                    : messageKind === "file"
                                      ? HusIcon.FileOutlined
                                      : messageKind === "image"
                                        ? HusIcon.PictureOutlined
                                        : messageKind === "emoji"
                                          ? HusIcon.SmileOutlined
                                          : HusIcon.MessageOutlined
    readonly property string summaryText: {
        if (messageKind === "shake")
            return qsTr("窗口抖动");
        if (messageKind === "file")
            return fileSizeText.length > 0
                    ? qsTr("文件：%1 · %2").arg(fileName).arg(fileSizeText)
                    : qsTr("文件：%1").arg(fileName);
        if (messageKind === "image")
            return messageText.length > 0
                    ? qsTr("图片：%1").arg(messageText)
                    : qsTr("图片：%1").arg(fileName);
        if (messageKind === "emoji")
            return qsTr("表情：%1").arg(messageText);
        return messageText;
    }

    implicitHeight: Math.max(76, messageSummary.implicitHeight + 50)
    Accessible.role: Accessible.StaticText
    Accessible.name: qsTr("%1，%2，%3").arg(senderName).arg(messageTime).arg(summaryText)

    Rectangle {
        anchors.fill: parent
        radius: HusTheme.Primary.radiusPrimaryLG
        color: delegateHover.hovered
               ? HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary,
                                         HusTheme.isDark ? 0.12 : 0.06)
               : "transparent"
        border.width: 1
        border.color: delegateHover.hovered
                      ? HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary,
                                                HusTheme.isDark ? 0.3 : 0.16)
                      : HusTheme.Primary.colorSplit
        Accessible.ignored: true
    }

    HusAvatar {
        id: senderAvatar

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 14
        anchors.topMargin: 14
        size: 38
        textSource: root.senderInitial
        colorBg: root.senderColor
        textSize: HusAvatar.Size_Auto
    }

    HusText {
        id: messageSender

        anchors.left: senderAvatar.right
        anchors.right: messageTimeLabel.left
        anchors.top: parent.top
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        anchors.topMargin: 13
        text: root.senderName
        color: HusTheme.Primary.colorTextBase
        elide: Text.ElideRight
        font.pixelSize: HusTheme.Primary.fontPrimarySize
        font.weight: Font.Medium
    }

    HusText {
        id: messageTimeLabel

        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: 14
        anchors.topMargin: 14
        text: root.messageTime
        color: HusTheme.Primary.colorTextQuaternary
        font.pixelSize: Math.max(12, HusTheme.Primary.fontPrimarySize - 2)
    }

    HusIconText {
        id: kindIndicator

        anchors.left: senderAvatar.right
        anchors.top: messageSender.bottom
        anchors.leftMargin: 12
        anchors.topMargin: 8
        width: 18
        height: 18
        iconSource: root.kindIcon
        iconSize: 16
        colorIcon: HusTheme.Primary.colorTextTertiary
        Accessible.ignored: true
    }

    HusText {
        id: messageSummary

        anchors.left: kindIndicator.right
        anchors.right: parent.right
        anchors.top: messageSender.bottom
        anchors.leftMargin: 8
        anchors.rightMargin: 14
        anchors.topMargin: 7
        text: root.summaryText
        color: HusTheme.Primary.colorTextSecondary
        wrapMode: Text.Wrap
        maximumLineCount: 3
        elide: Text.ElideRight
        font.pixelSize: HusTheme.Primary.fontPrimarySize
    }

    HoverHandler {
        id: delegateHover
    }
}
