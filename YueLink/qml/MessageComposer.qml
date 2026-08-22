pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Dialogs
import HuskarUI.Basic
import "EmojiCatalog.mjs" as EmojiCatalog

Item {
    id: root

    property string conversationId: ""
    property bool sendEnabled: false
    property bool filesEnabled: false
    property bool draggingFiles: false
    property var drafts: ({})
    property string draftConversationId: ""
    property color inputSurfaceColor: HusTheme.HusCard.colorBg

    signal imagesSelected(var imageUrls)
    signal filesSelected(var fileUrls)
    signal historyRequested()

    function submitMessage(): void {
        const content = composer.text.trim();
        if (content.length === 0 || conversationId.length === 0 || !sendEnabled)
            return;

        if (!LanChat.sendMessage(conversationId, content))
            return;

        composer.clear();
        drafts[draftConversationId] = "";
    }

    function switchDraft(nextConversationId: string): void {
        if (draftConversationId.length > 0)
            drafts[draftConversationId] = composer.text;

        draftConversationId = nextConversationId;
        composer.text = nextConversationId.length > 0
                && drafts[nextConversationId] !== undefined
                ? drafts[nextConversationId]
                : "";
    }

    function sendClipboardImages(): bool {
        if (!filesEnabled)
            return false;
        const imageUrls = LanChat.clipboardImageUrls();
        if (imageUrls.length === 0)
            return false;

        imagesSelected(imageUrls);
        return true;
    }

    function isImageUrl(url): bool {
        const path = url.toString().toLowerCase();
        return [".png", ".jpg", ".jpeg", ".bmp", ".gif", ".webp"]
                .some(extension => path.endsWith(extension));
    }

    onConversationIdChanged: switchDraft(conversationId)

    Component.onDestruction: {
        if (draftConversationId.length > 0)
            drafts[draftConversationId] = composer.text;
    }

    onSendEnabledChanged: {
        if (!sendEnabled)
            draggingFiles = false;
    }

    onFilesEnabledChanged: {
        if (!filesEnabled)
            draggingFiles = false;
    }

    implicitHeight: 178

    Rectangle {
        anchors.fill: parent
        radius: HusTheme.Primary.radiusPrimaryLG
        color: root.inputSurfaceColor
        border.width: 1
        border.color: HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary,
                                              HusTheme.isDark ? 0.34 : 0.2)
        Accessible.ignored: true
    }

    Row {
        id: composerTools

        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 8
        spacing: 2

        HusIconButton {
            id: emojiPickerButton

            width: 38
            height: 38
            padding: 0
            type: HusButton.Type_Text
            effectEnabled: false
            colorBg: "transparent"
            colorBorder: "transparent"
            borderWidth: 0
            iconSource: HusIcon.SmileOutlined
            iconSize: 19
            colorIcon: !enabled ? HusTheme.Primary.colorTextQuaternary
                                : visualFocus ? HusTheme.Primary.colorPrimary
                                : hovered ? HusTheme.Primary.colorTextBase : HusTheme.Primary.colorTextTertiary
            enabled: root.sendEnabled
            contentDescription: qsTr("发送表情")
            onClicked: emojiPopup.open()

            HusToolTip {
                visible: emojiPickerButton.hovered || emojiPickerButton.visualFocus
                position: HusToolTip.Position_Bottom
                text: emojiPickerButton.contentDescription
            }
        }

        HusIconButton {
            id: filePickerButton

            width: 38
            height: 38
            padding: 0
            type: HusButton.Type_Text
            effectEnabled: false
            colorBg: "transparent"
            colorBorder: "transparent"
            borderWidth: 0
            iconSource: HusIcon.FolderOpenOutlined
            iconSize: 19
            colorIcon: !enabled ? HusTheme.Primary.colorTextQuaternary
                                : visualFocus ? HusTheme.Primary.colorPrimary
                                : hovered ? HusTheme.Primary.colorTextBase : HusTheme.Primary.colorTextTertiary
            enabled: root.filesEnabled
            contentDescription: root.filesEnabled
                                ? qsTr("发送文件")
                                : qsTr("当前会话暂不支持发送文件")
            onClicked: fileDialog.open()

            HusToolTip {
                visible: filePickerButton.hovered || filePickerButton.visualFocus
                position: HusToolTip.Position_Bottom
                text: filePickerButton.contentDescription
            }
        }

        HusIconButton {
            id: imagePickerButton

            width: 38
            height: 38
            padding: 0
            type: HusButton.Type_Text
            effectEnabled: false
            colorBg: "transparent"
            colorBorder: "transparent"
            borderWidth: 0
            iconSource: HusIcon.PictureOutlined
            iconSize: 19
            colorIcon: !enabled ? HusTheme.Primary.colorTextQuaternary
                                : visualFocus ? HusTheme.Primary.colorPrimary
                                : hovered ? HusTheme.Primary.colorTextBase : HusTheme.Primary.colorTextTertiary
            enabled: root.filesEnabled
            contentDescription: root.filesEnabled
                                ? qsTr("发送图片")
                                : qsTr("当前会话暂不支持发送图片")
            onClicked: imageFileDialog.open()

            HusToolTip {
                visible: imagePickerButton.hovered || imagePickerButton.visualFocus
                position: HusToolTip.Position_Bottom
                text: imagePickerButton.contentDescription
            }
        }
    }

    HusIconButton {
        id: historyButton

        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: 10
        anchors.topMargin: 8
        width: 38
        height: 38
        padding: 0
        type: HusButton.Type_Text
        effectEnabled: false
        colorBg: "transparent"
        colorBorder: "transparent"
        borderWidth: 0
        iconSource: HusIcon.HistoryOutlined
        iconSize: 19
        colorIcon: !enabled ? HusTheme.Primary.colorTextQuaternary
                            : visualFocus ? HusTheme.Primary.colorPrimary
                            : hovered ? HusTheme.Primary.colorTextBase : HusTheme.Primary.colorTextTertiary
        enabled: root.conversationId.length > 0
        contentDescription: qsTr("聊天记录")
        onClicked: root.historyRequested()

        HusToolTip {
            visible: historyButton.hovered || historyButton.visualFocus
            position: HusToolTip.Position_Bottom
            text: historyButton.contentDescription
        }
    }

    HusText {
        anchors.left: composerTools.right
        anchors.right: historyButton.left
        anchors.top: parent.top
        anchors.leftMargin: 12
        anchors.rightMargin: 8
        anchors.topMargin: 17
        visible: composer.text.length >= 1600
        text: qsTr("%1/2000 个字符").arg(composer.text.length)
        color: HusTheme.Primary.colorTextQuaternary
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignRight
        font.pixelSize: HusTheme.Primary.fontPrimarySize
    }

    HusTextArea {
        id: composer

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: composerTools.bottom
        anchors.bottom: parent.bottom
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        anchors.bottomMargin: 56
        maxLength: 2000
        enabled: root.sendEnabled
        colorBg: "transparent"
        colorBorder: "transparent"
        radiusBg.all: 0
        placeholderText: root.sendEnabled
                         ? qsTr("输入消息，按 Ctrl+Enter 发送")
                         : root.conversationId.length === 0
                           ? qsTr("请先选择一个会话")
                           : qsTr("联系人离线，暂时无法发送消息")
        contentDescription: qsTr("消息输入框")
        textArea.wrapMode: TextEdit.Wrap
        textArea.Keys.onPressed: event => {
            if ((event.modifiers & Qt.ControlModifier) && event.key === Qt.Key_V
                    && root.sendClipboardImages()) {
                event.accepted = true;
            }
        }
    }

    HusButton {
        id: sendButton

        anchors.right: parent.right
        anchors.rightMargin: 12
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 12
        width: 96
        height: 40
        type: HusButton.Type_Primary
        text: qsTr("发送")
        enabled: root.sendEnabled && composer.text.trim().length > 0
        contentDescription: qsTr("发送消息")
        onClicked: root.submitMessage()
    }

    Shortcut {
        sequence: "Ctrl+Return"
        enabled: root.sendEnabled && composer.text.trim().length > 0
        onActivated: root.submitMessage()
    }

    Shortcut {
        sequence: "Ctrl+Enter"
        enabled: root.sendEnabled && composer.text.trim().length > 0
        onActivated: root.submitMessage()
    }

    Shortcut {
        sequences: [StandardKey.Find]
        enabled: root.conversationId.length > 0
        onActivated: root.historyRequested()
    }

    Rectangle {
        anchors.fill: parent
        visible: root.draggingFiles
        z: 10
        radius: HusTheme.Primary.radiusPrimary
        color: HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, HusTheme.isDark ? 0.28 : 0.16)
        border.width: 2
        border.color: HusTheme.Primary.colorPrimary

        HusText {
            anchors.centerIn: parent
            text: qsTr("释放鼠标发送文件")
            color: HusTheme.Primary.colorPrimary
            font.pixelSize: HusTheme.Primary.fontPrimarySizeHeading5
            font.weight: Font.Medium
        }
    }

    DropArea {
        id: fileDropArea

        anchors.fill: parent
        enabled: root.filesEnabled
        onEntered: drag => {
            const acceptable = drag.hasUrls;
            drag.accepted = acceptable;
            root.draggingFiles = acceptable;
        }
        onExited: root.draggingFiles = false
        onDropped: drop => {
            root.draggingFiles = false;
            if (!drop.hasUrls) {
                drop.accepted = false;
                return;
            }
            drop.acceptProposedAction();
            const imageUrls = [];
            const fileUrls = [];
            for (const url of drop.urls) {
                if (root.isImageUrl(url))
                    imageUrls.push(url);
                else
                    fileUrls.push(url);
            }
            if (imageUrls.length > 0)
                root.imagesSelected(imageUrls);
            if (fileUrls.length > 0)
                root.filesSelected(fileUrls);
        }
    }

    FileDialog {
        id: fileDialog

        title: qsTr("选择要发送的文件，可多选")
        fileMode: FileDialog.OpenFiles
        onAccepted: root.filesSelected(selectedFiles)
    }

    FileDialog {
        id: imageFileDialog

        title: qsTr("选择要发送的图片，可多选")
        fileMode: FileDialog.OpenFiles
        nameFilters: [qsTr("图片文件 (*.png *.jpg *.jpeg *.bmp *.gif *.webp)")]
        onAccepted: root.imagesSelected(selectedFiles)
    }

    HusPopup {
        id: emojiPopup

        x: 12
        y: -height - 8
        width: 368
        height: 316
        contentItem: GridView {
            id: emojiGrid

            cellWidth: 48
            cellHeight: 48
            model: EmojiCatalog.emojis
            boundsBehavior: Flickable.StopAtBounds
            clip: true
            ScrollBar.vertical: HusScrollBar { }

            delegate: HusButton {
                id: emojiButton

                required property var modelData
                readonly property url imageSource: Qt.resolvedUrl(
                                                       EmojiCatalog.resourcePrefix
                                                       + String(modelData.file))

                width: emojiGrid.cellWidth
                height: emojiGrid.cellHeight
                padding: 5
                type: HusButton.Type_Text
                enabled: root.sendEnabled
                contentDescription: qsTr("发送表情：%1").arg(modelData.name)

                Image {
                    anchors.centerIn: parent
                    width: 34
                    height: 34
                    source: emojiButton.imageSource
                    sourceSize.width: 34
                    sourceSize.height: 34
                    fillMode: Image.PreserveAspectFit
                    asynchronous: true
                    Accessible.ignored: true
                }

                HusToolTip {
                    visible: emojiButton.hovered
                    text: emojiButton.modelData.name
                    position: HusToolTip.Position_Top
                }

                onClicked: {
                    if (LanChat.sendEmoji(root.conversationId,
                                          EmojiCatalog.packageId,
                                          modelData.emojiId,
                                          modelData.name)) {
                        emojiPopup.close();
                    }
                }
            }
        }
    }
}
