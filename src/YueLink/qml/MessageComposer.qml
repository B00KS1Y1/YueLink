import QtQuick
import QtQuick.Dialogs
import HuskarUI.Basic

Item {
    id: root

    property string conversationId: ""
    property bool sendEnabled: false
    property bool filesEnabled: false
    property bool draggingFiles: false
    property var drafts: ({})
    property string draftConversationId: ""
    readonly property color surfaceColor: HusThemeFunctions.alpha(
                                              HusTheme.Primary.colorBgContainer,
                                              HusTheme.isDark ? 0.8 : 0.88)
    readonly property color inputSurfaceColor: HusThemeFunctions.alpha(
                                                   HusTheme.Primary.colorBgBase,
                                                   HusTheme.isDark ? 0.54 : 0.62)

    signal imagesSelected(var imageUrls)
    signal filesSelected(var fileUrls)

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

    implicitHeight: 164

    Rectangle {
        anchors.fill: parent
        radius: 16
        color: root.surfaceColor
        border.width: 1
        border.color: HusTheme.Primary.colorBorderSecondary
        Accessible.ignored: true
    }

    Row {
        id: composerTools

        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.top: parent.top
        anchors.topMargin: 8
        spacing: 2

        HusIconButton {
            width: 40
            height: 40
            padding: 0
            type: HusButton.Type_Text
            iconSource: HusIcon.PictureOutlined
            iconSize: 20
            enabled: root.filesEnabled
            contentDescription: root.filesEnabled
                                ? qsTr("发送图片")
                                : qsTr("当前会话暂不支持发送图片")
            onClicked: imageFileDialog.open()
        }

        HusIconButton {
            width: 40
            height: 40
            padding: 0
            type: HusButton.Type_Text
            iconSource: HusIcon.FolderOpenOutlined
            iconSize: 20
            enabled: root.filesEnabled
            contentDescription: root.filesEnabled
                                ? qsTr("发送文件")
                                : qsTr("当前会话暂不支持发送文件")
            onClicked: fileDialog.open()
        }

        HusIconButton {
            width: 40
            height: 40
            padding: 0
            type: HusButton.Type_Text
            iconSource: HusIcon.SmileOutlined
            iconSize: 20
            enabled: root.sendEnabled
            contentDescription: qsTr("发送表情")
            onClicked: emojiPopup.open()
        }
    }

    HusText {
        anchors.left: composerTools.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: 12
        anchors.rightMargin: 20
        anchors.topMargin: 18
        visible: composer.text.length >= 1600
        text: qsTr("%1/2000 个字符").arg(composer.text.length)
        color: HusTheme.Primary.colorTextTertiary
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignRight
        font.pixelSize: HusTheme.Primary.fontPrimarySize
    }

    HusTextArea {
        id: composer

        anchors.left: parent.left
        anchors.right: sendButton.left
        anchors.top: composerTools.bottom
        anchors.bottom: parent.bottom
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        anchors.bottomMargin: 12
        maxLength: 2000
        enabled: root.sendEnabled
        colorBg: root.inputSurfaceColor
        colorBorder: HusTheme.Primary.colorBorderSecondary
        radiusBg.all: 12
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

    HusIconButton {
        id: sendButton

        anchors.right: parent.right
        anchors.rightMargin: 12
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 12
        width: 92
        height: 42
        type: HusButton.Type_Primary
        text: qsTr("发送")
        iconSource: HusIcon.SendOutlined
        iconSize: 17
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

    Rectangle {
        anchors.fill: parent
        visible: root.draggingFiles
        z: 10
        radius: 16
        color: HusTheme.Primary.colorPrimaryBg
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

        x: 92
        y: 48
        width: 240
        height: 64
        contentItem: Row {
            spacing: 6

            Repeater {
                model: ["😀", "😂", "😍", "👍", "🎉"]

                HusButton {
                    required property string modelData

                    width: 40
                    height: 40
                    type: HusButton.Type_Text
                    text: modelData
                    onClicked: {
                        LanChat.sendEmoji(root.conversationId, "builtin",
                                          modelData, modelData);
                        emojiPopup.close();
                    }
                }
            }
        }
    }
}
