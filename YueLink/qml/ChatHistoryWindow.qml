pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic
import HuskarUI.Basic

HusWindow {
    id: root

    property Window ownerWindow: null
    property string conversationTitle: qsTr("聊天记录")
    property bool closePending: false
    readonly property string searchKeyword: LanChat.messageHistorySearchText.trim()
    readonly property bool hasSearchFilter: searchKeyword.length > 0
                                             || LanChat.messageHistoryCategory !== "all"

    signal closeRequested()

    function requestClose(): void {
        if (closePending)
            return;
        closePending = true;
        LanChat.messageHistorySearchText = "";
        LanChat.messageHistoryCategory = "all";
        closeRequested();
    }

    function focusSearch(): void {
        searchInput.forceActiveFocus();
        searchInput.selectAll();
    }

    width: 820
    height: 700
    minimumWidth: 640
    minimumHeight: 480
    transientParent: ownerWindow
    modality: Qt.NonModal
    title: qsTr("%1 - 聊天记录").arg(conversationTitle)
    color: HusTheme.Primary.colorBgBase
    captionBar.height: 42
    captionBar.color: HusTheme.Primary.colorBgBase
    captionBar.showWinIcon: false
    captionBar.winTitle: conversationTitle
    captionBar.closeCallback: () => root.requestClose()

    Component.onCompleted: {
        LanChat.messageHistorySearchText = "";
        LanChat.messageHistoryCategory = "all";
        if (ownerWindow) {
            x = ownerWindow.x + Math.round((ownerWindow.width - width) / 2);
            y = ownerWindow.y + Math.round((ownerWindow.height - height) / 2);
        }
        Qt.callLater(() => root.focusSearch());
    }

    onClosing: close => {
        close.accepted = false;
        root.requestClose();
    }

    Rectangle {
        id: contentSurface

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.topMargin: root.captionBar.height
        color: HusTheme.Primary.colorBgBase

        HusInput {
            id: searchInput

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.leftMargin: 24
            anchors.rightMargin: 24
            anchors.topMargin: 18
            height: 38
            iconSource: HusIcon.SearchOutlined
            iconSize: 17
            iconPosition: HusInput.Position_Left
            clearEnabled: "active"
            type: HusInput.Type_Filled
            colorBg: HusThemeFunctions.alpha(HusTheme.Primary.colorTextBase,
                                             HusTheme.isDark ? 0.08 : 0.05)
            verticalAlignment: TextInput.AlignVCenter
            placeholderText: qsTr("搜索聊天记录")
            contentDescription: qsTr("搜索发送者、消息内容或文件名")
            text: LanChat.messageHistorySearchText
            onTextChanged: LanChat.messageHistorySearchText = text
        }

        Item {
            id: categoryBar

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: searchInput.bottom
            anchors.leftMargin: 24
            anchors.rightMargin: 24
            anchors.topMargin: 12
            height: 46

            Row {
                anchors.left: parent.left
                anchors.top: parent.top
                height: parent.height
                spacing: 10

                Repeater {
                    model: [
                        { "label": qsTr("全部"), "value": "all" },
                        { "label": qsTr("图片/视频"), "value": "media" },
                        { "label": qsTr("文件"), "value": "file" }
                    ]

                    delegate: HusButton {
                        id: categoryButton

                        required property var modelData
                        readonly property bool selected:
                            LanChat.messageHistoryCategory === modelData.value

                        width: modelData.value === "media" ? 104 : 72
                        height: categoryBar.height
                        padding: 0
                        type: HusButton.Type_Text
                        effectEnabled: false
                        colorBg: "transparent"
                        colorBorder: "transparent"
                        borderWidth: 0
                        colorText: selected
                                   ? HusTheme.Primary.colorPrimary
                                   : HusTheme.Primary.colorTextBase
                        text: modelData.label
                        contentDescription: qsTr("筛选%1聊天记录").arg(modelData.label)
                        onClicked: LanChat.messageHistoryCategory = modelData.value

                        Rectangle {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            height: 2
                            radius: 1
                            visible: categoryButton.selected
                            color: HusTheme.Primary.colorPrimary
                            Accessible.ignored: true
                        }
                    }
                }
            }

            HusText {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("%1 条记录").arg(historyList.count)
                color: HusTheme.Primary.colorTextQuaternary
                font.pixelSize: Math.max(12, HusTheme.Primary.fontPrimarySize - 2)
            }
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: categoryBar.bottom
            height: 1
            color: HusTheme.Primary.colorSplit
            Accessible.ignored: true
        }

        ListView {
            id: historyList

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: categoryBar.bottom
            anchors.bottom: parent.bottom
            anchors.leftMargin: 24
            anchors.rightMargin: 24
            anchors.topMargin: 17
            anchors.bottomMargin: 18
            model: LanChat.messageHistory
            spacing: 8
            boundsBehavior: Flickable.StopAtBounds
            reuseItems: true
            clip: true
            ScrollBar.vertical: HusScrollBar { }

            delegate: HistoryMessageDelegate {
                width: historyList.width
            }
        }

        Column {
            anchors.centerIn: historyList
            width: Math.min(360, Math.max(0, historyList.width - 48))
            spacing: 14
            visible: historyList.count === 0

            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                width: 76
                height: 76
                radius: 38
                color: "transparent"
                border.width: 2
                border.color: HusTheme.Primary.colorTextQuaternary
                Accessible.ignored: true

                HusIconText {
                    anchors.centerIn: parent
                    iconSource: root.hasSearchFilter
                                ? HusIcon.SearchOutlined
                                : HusIcon.MessageOutlined
                    iconSize: 34
                    colorIcon: HusTheme.Primary.colorTextTertiary
                }
            }

            HusText {
                width: parent.width
                text: root.hasSearchFilter
                      ? qsTr("没有找到相关记录")
                      : qsTr("暂无消息")
                color: HusTheme.Primary.colorTextBase
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: HusTheme.Primary.fontPrimarySizeHeading5
                font.weight: Font.Medium
            }

            HusText {
                width: parent.width
                visible: root.hasSearchFilter
                text: root.searchKeyword.length > 0
                      ? qsTr("没有与“%1”匹配的聊天记录").arg(root.searchKeyword)
                      : qsTr("当前分类中没有聊天记录")
                color: HusTheme.Primary.colorTextTertiary
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap
                font.pixelSize: HusTheme.Primary.fontPrimarySize
            }
        }
    }

    Shortcut {
        sequences: [StandardKey.Find]
        onActivated: root.focusSearch()
    }

    Shortcut {
        sequence: "Escape"
        onActivated: root.requestClose()
    }
}
