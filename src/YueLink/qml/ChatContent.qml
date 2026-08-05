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
    readonly property color headerSurfaceColor: HusThemeFunctions.alpha(
                                                      HusTheme.Primary.colorBgContainer,
                                                      HusTheme.isDark ? 0.7 : 0.78)
    readonly property color messageBubbleColor: HusThemeFunctions.alpha(
                                                     HusTheme.Primary.colorBgContainer,
                                                     HusTheme.isDark ? 0.84 : 0.92)

    signal cancelFileRequested(string messageId)
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

    function isImageFile(fileName: string): bool {
        const extensionIndex = fileName.lastIndexOf(".");
        if (extensionIndex < 0)
            return false;

        const extension = fileName.slice(extensionIndex + 1).toLowerCase();
        return ["png", "jpg", "jpeg", "bmp", "gif", "webp"].indexOf(extension) >= 0;
    }

    function deliverySummary(status: string, kind: string, fromMe: bool,
                             time: string, progress: real,
                             deliveredCount: int, totalCount: int): string {
        if (status === "failed")
            return fromMe ? qsTr("%1 · 发送失败").arg(time)
                          : qsTr("%1 · 接收失败").arg(time);
        if (status === "cancelled")
            return qsTr("%1 · 已取消").arg(time);
        if (status === "transferring")
            return qsTr("%1 · 正在传输 %2%")
                    .arg(time).arg(Math.round(progress * 100));
        if (status === "receiving")
            return qsTr("%1 · 正在接收 %2%")
                    .arg(time).arg(Math.round(progress * 100));
        if (kind === "file" && status === "received")
            return qsTr("%1 · 已保存到下载目录").arg(time);
        if (kind === "file" && status === "sent")
            return qsTr("%1 · 已发送").arg(time);
        if (fromMe && totalCount > 0) {
            if (status === "sent" || status === "partial")
                return qsTr("%1 · 已发送 %2/%3")
                        .arg(time).arg(deliveredCount).arg(totalCount);
            return qsTr("%1 · 待发送 %2/%3")
                    .arg(time).arg(deliveredCount).arg(totalCount);
        }
        if (status === "sending" || status === "pending")
            return qsTr("%1 · 发送中").arg(time);
        return time;
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
        height: 76
        radius: 14
        color: root.headerSurfaceColor
        border.width: 0

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
                    color: HusTheme.Primary.colorTextBase
                    elide: Text.ElideRight
                    font.pixelSize: HusTheme.Primary.fontPrimarySizeHeading5
                    font.weight: Font.Medium
                }

                HusText {
                    width: parent.width
                    text: root.conversationStatus
                    color: HusTheme.Primary.colorTextSecondary
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

            radius: 12
            color: root.headerSurfaceColor
            border.width: 1
            border.color: HusTheme.Primary.colorBorderSecondary

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
                color: HusTheme.Primary.colorTextTertiary
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
        spacing: 8
        boundsBehavior: Flickable.StopAtBounds
        reuseItems: true
        clip: true
        ScrollBar.vertical: HusScrollBar { }

        delegate: Item {
            id: messageDelegate

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
            required property int deliveredCount
            required property int totalRecipientCount

            readonly property bool fileTransferActive:
                deliveryStatus === "transferring"
                || deliveryStatus === "receiving"
            readonly property bool fileTransferComplete:
                deliveryStatus === "sent"
                || deliveryStatus === "received"
            readonly property bool imageFile:
                messageKind === "file" && root.isImageFile(fileName)
            readonly property bool imagePreviewAvailable:
                imageFile && fileUrl.toString().length > 0
                && (fromMe || fileTransferComplete)

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
                width: messageDelegate.imageFile
                       ? Math.min(360, Math.max(220, messageList.width * 0.58))
                       : messageDelegate.messageKind === "file"
                       ? Math.min(Math.max(260, messageTextItem.implicitWidth + 28),
                                  Math.max(260, messageList.width * 0.58))
                       : Math.min(messageTextItem.implicitWidth + 28,
                                  Math.max(220, messageList.width * 0.58))
                spacing: 5

                HusText {
                    width: parent.width
                    visible: root.conversationKind === "group"
                             && !messageDelegate.fromMe
                    text: messageDelegate.senderName
                    color: HusTheme.Primary.colorTextSecondary
                    elide: Text.ElideRight
                    font.pixelSize: Math.max(11,
                                             HusTheme.Primary.fontPrimarySize - 1)
                    font.weight: Font.Medium
                }

                Rectangle {
                    width: parent.width
                    height: messagePreview.status === Image.Ready
                            ? messagePreview.height + messageTextItem.implicitHeight
                              + (messageDelegate.messageKind === "file" ? 44 : 28)
                            : messageTextItem.implicitHeight
                            + (messageDelegate.messageKind === "file" ? 36 : 20)
                    radius: 14
                    color: messageDelegate.fromMe
                           ? HusTheme.Primary.colorPrimaryBg
                           : root.messageBubbleColor
                    border.width: messageDelegate.fromMe ? 0 : 1
                    border.color: HusTheme.Primary.colorBorderSecondary

                    HusText {
                        id: messageTextItem

                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: messagePreview.status === Image.Ready
                                     ? messagePreview.bottom
                                     : parent.top
                        anchors.bottom: messageDelegate.messageKind === "file"
                                        ? fileProgressTrack.top
                                        : parent.bottom
                        anchors.leftMargin: 14
                        anchors.rightMargin: 14
                        anchors.topMargin: messagePreview.status === Image.Ready ? 8 : 10
                        anchors.bottomMargin: messageDelegate.messageKind === "file" ? 8 : 10
                        text: messageDelegate.messageKind === "file"
                              ? (messageDelegate.imageFile
                                 ? qsTr("图片：%1\n%2").arg(messageDelegate.fileName)
                                                        .arg(messageDelegate.fileSizeText)
                                 : qsTr("文件：%1\n%2").arg(messageDelegate.fileName)
                                                        .arg(messageDelegate.fileSizeText))
                              : messageDelegate.messageText
                        color: HusTheme.Primary.colorTextBase
                        wrapMode: Text.Wrap
                        font.pixelSize: HusTheme.Primary.fontPrimarySize
                    }

                    Image {
                        id: messagePreview

                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10
                        anchors.topMargin: 10
                        height: status === Image.Ready ? 220 : 0
                        source: messageDelegate.imagePreviewAvailable
                                ? messageDelegate.fileUrl
                                : ""
                        sourceSize.width: 560
                        sourceSize.height: 440
                        asynchronous: true
                        mipmap: true
                        fillMode: Image.PreserveAspectFit
                        visible: status === Image.Ready
                        Accessible.name: qsTr("图片预览：%1").arg(messageDelegate.fileName)
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
                    text: root.deliverySummary(
                              messageDelegate.deliveryStatus,
                              messageDelegate.messageKind,
                              messageDelegate.fromMe,
                              messageDelegate.messageTime,
                              messageDelegate.fileProgress,
                              messageDelegate.deliveredCount,
                              messageDelegate.totalRecipientCount)
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
            color: HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary,
                                           HusTheme.isDark ? 0.2 : 0.12)
            border.width: 1
            border.color: HusTheme.Primary.colorBorderSecondary
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
                  ? root.messageSearchKeyword.length > 0
                    ? qsTr("没有找到消息")
                    : qsTr("开始聊天")
                  : qsTr("发现身边的好友")
            color: HusTheme.Primary.colorTextBase
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
            color: HusTheme.Primary.colorTextTertiary
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
