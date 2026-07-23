import QtQuick
import QtQuick.Dialogs
import HuskarUI.Basic

Rectangle {
    id: root

    property bool sendEnabled: false
    property string errorText: ""
    property bool draggingFiles: false

    signal sendRequested(string message)
    signal filesSelected(var fileUrls)

    function submitMessage(): void {
        const content = composer.text.trim();
        if (content.length === 0)
            return;

        sendRequested(content);
        composer.clear();
    }

    onSendEnabledChanged: {
        if (!sendEnabled)
            draggingFiles = false;
    }

    implicitHeight: 176
    color: HusTheme.Primary.colorBgContainer

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 1
        color: HusTheme.Primary.colorBorderSecondary
    }

    Row {
        id: composerTools

        anchors.left: parent.left
        anchors.leftMargin: 18
        anchors.top: parent.top
        anchors.topMargin: 10
        spacing: 2

        HusIconButton {
            width: 36
            height: 36
            padding: 0
            type: HusButton.Type_Text
            iconSource: HusIcon.SmileOutlined
            iconSize: 20
            contentDescription: qsTr("选择表情")
        }

        HusIconButton {
            width: 36
            height: 36
            padding: 0
            type: HusButton.Type_Text
            iconSource: HusIcon.PictureOutlined
            iconSize: 20
            enabled: root.sendEnabled
            contentDescription: qsTr("发送图片")
            onClicked: imageFileDialog.open()
        }

        HusIconButton {
            width: 36
            height: 36
            padding: 0
            type: HusButton.Type_Text
            iconSource: HusIcon.FolderOpenOutlined
            iconSize: 20
            enabled: root.sendEnabled
            contentDescription: qsTr("发送文件")
            onClicked: fileDialog.open()
        }

        HusIconButton {
            width: 36
            height: 36
            padding: 0
            type: HusButton.Type_Text
            iconSource: HusIcon.ScissorOutlined
            iconSize: 20
            contentDescription: qsTr("截图")
        }
    }

    HusText {
        anchors.left: composerTools.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: 12
        anchors.rightMargin: 20
        anchors.topMargin: 18
        visible: root.errorText.length > 0
        text: root.errorText
        color: "#D84A4A"
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
        anchors.leftMargin: 18
        anchors.rightMargin: 18
        anchors.bottomMargin: 18
        maxLength: 2000
        enabled: root.sendEnabled
        colorBg: "transparent"
        colorBorder: "transparent"
        placeholderText: root.sendEnabled
                         ? qsTr("输入消息，按 Ctrl+Enter 发送")
                         : qsTr("好友离线，暂时无法发送消息")
        contentDescription: qsTr("消息输入框")
        textArea.wrapMode: TextEdit.Wrap
    }

    HusIconButton {
        id: sendButton

        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        width: 94
        height: 38
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
        radius: 10
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
        enabled: root.sendEnabled
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
            root.filesSelected(drop.urls);
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
        onAccepted: root.filesSelected(selectedFiles)
    }
}
