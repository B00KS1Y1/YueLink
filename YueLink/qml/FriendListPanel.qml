pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import HuskarUI.Basic

Item {
    id: root

    property string selectedConversationId: ""
    property bool contactsMode: false
    property color searchSurfaceColor: HusTheme.Primary.colorFillTertiary
    readonly property bool groupContactsMode: contactsMode
                                               && contactTypeSelector.currentIndex === 1
    readonly property string searchKeyword: (contactsMode
                                              ? groupContactsMode
                                                ? LanChat.groupSearchText
                                                : LanChat.peerSearchText
                                              : LanChat.conversationSearchText).trim()

    signal conversationSelected(string conversationId)
    signal createGroupRequested()

    Item {
        id: searchSection

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 60

        RowLayout {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 14
            anchors.rightMargin: 14
            spacing: 8

            HusInput {
                id: searchInput

                Layout.fillWidth: true
                Layout.preferredHeight: 36
                iconSource: HusIcon.SearchOutlined
                iconSize: 16
                iconPosition: HusInput.Position_Left
                clearEnabled: "active"
                type: HusInput.Type_Filled
                colorBg: root.searchSurfaceColor
                verticalAlignment: TextInput.AlignVCenter
                placeholderText: qsTr("搜索")
                contentDescription: placeholderText
                text: root.contactsMode
                      ? root.groupContactsMode
                        ? LanChat.groupSearchText
                        : LanChat.peerSearchText
                      : LanChat.conversationSearchText
                onTextChanged: {
                    if (!root.contactsMode) {
                        LanChat.conversationSearchText = text;
                    } else if (root.groupContactsMode) {
                        LanChat.groupSearchText = text;
                    } else {
                        LanChat.peerSearchText = text;
                    }
                }
            }

            HusIconButton {
                id: quickActionsButton

                Layout.preferredWidth: 36
                Layout.preferredHeight: 36
                padding: 0
                type: HusButton.Type_Filled
                iconSource: HusIcon.PlusOutlined
                iconSize: 18
                contentDescription: qsTr("打开快捷操作菜单")
                onClicked: quickActionsMenu.open()

                HusContextMenu {
                    id: quickActionsMenu

                    x: quickActionsButton.width - implicitWidth
                    y: quickActionsButton.height + 4
                    defaultMenuWidth: 148
                    initModel: [
                        {
                            key: "createGroup",
                            label: qsTr("创建群聊"),
                            iconSource: HusIcon.UsergroupAddOutlined
                        }
                    ]
                    onClickMenu: (deep, key) => {
                        if (deep === 0 && key === "createGroup")
                            root.createGroupRequested();
                    }
                }
            }
        }
    }

    Shortcut {
        sequence: "Ctrl+K"
        onActivated: searchInput.forceActiveFocus()
    }

    Item {
        id: contactTypeSection

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: searchSection.bottom
        height: visible ? 46 : 0
        visible: root.contactsMode

        HusSegmented {
            id: contactTypeSelector

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 14
            anchors.rightMargin: 14
            height: 34
            block: true
            focusPolicy: Qt.StrongFocus
            options: [
                { "label": qsTr("好友"), "value": "friends" },
                { "label": qsTr("群聊"), "value": "groups" }
            ]
            Accessible.name: qsTr("联系人类型")
            Keys.onLeftPressed: currentIndex = 0
            Keys.onRightPressed: currentIndex = 1
        }
    }

    Item {
        id: listHeader

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: root.contactsMode
                     ? contactTypeSection.bottom
                     : searchSection.bottom
        height: 38

        HusText {
            anchors.left: parent.left
            anchors.leftMargin: 16
            anchors.verticalCenter: parent.verticalCenter
            text: root.contactsMode
                  ? root.groupContactsMode ? qsTr("群聊") : qsTr("好友")
                  : qsTr("消息")
            color: HusTheme.Primary.colorTextBase
            font.pixelSize: HusTheme.Primary.fontPrimarySize
            font.weight: Font.Medium
        }

        HusText {
            anchors.right: parent.right
            anchors.rightMargin: 16
            anchors.verticalCenter: parent.verticalCenter
            text: root.contactsMode
                  ? root.groupContactsMode
                    ? qsTr("%1 个群聊").arg(conversationList.count)
                    : qsTr("%1 在线").arg(LanChat.onlineCount)
                  : LanChat.totalUnreadCount > 0
                    ? qsTr("%1 未读").arg(LanChat.totalUnreadCount)
                    : ""
            color: HusTheme.Primary.colorTextQuaternary
            font.pixelSize: Math.max(11, HusTheme.Primary.fontPrimarySize - 1)
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            height: 1
            color: HusTheme.Primary.colorSplit
            Accessible.ignored: true
        }
    }

    ListView {
        id: conversationList

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: listHeader.bottom
        anchors.bottom: parent.bottom
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        anchors.bottomMargin: 10
        model: root.contactsMode
               ? root.groupContactsMode ? LanChat.groups : LanChat.peers
               : LanChat.conversations
        currentIndex: -1
        boundsBehavior: Flickable.StopAtBounds
        reuseItems: true
        clip: true
        ScrollBar.vertical: HusScrollBar { }

        delegate: Item {
            id: conversationDelegate

            required property int index
            required property string itemId
            required property string itemKind
            required property string title
            required property string initial
            required property string statusText
            required property string lastMessage
            required property string lastTime
            required property color avatarColor
            required property bool online
            required property int unread
            required property string peerId
            required property int memberCount
            required property int onlineCount
            readonly property bool selected: root.selectedConversationId
                                             === conversationDelegate.itemId

            width: conversationList.width
            height: 74

            Rectangle {
                anchors.fill: parent
                anchors.margins: 4
                radius: HusTheme.Primary.radiusPrimary
                color: conversationDelegate.selected
                       ? HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, HusTheme.isDark ? 0.28 : 0.16)
                       : conversationMouse.containsMouse
                         ? HusTheme.Primary.colorFillSecondary
                         : "transparent"
                border.width: conversationMouse.activeFocus ? 1 : 0
                border.color: HusTheme.Primary.colorPrimary

                Behavior on color {
                    enabled: HusTheme.animationEnabled
                    ColorAnimation {
                        duration: HusTheme.Primary.durationFast
                    }
                }
            }

            Rectangle {
                anchors.left: parent.left
                anchors.leftMargin: 4
                anchors.verticalCenter: parent.verticalCenter
                width: 3
                height: 28
                radius: width * 0.5
                visible: conversationDelegate.selected
                color: HusTheme.Primary.colorPrimary
                Accessible.ignored: true
            }

            HusAvatar {
                id: conversationAvatar

                anchors.left: parent.left
                anchors.leftMargin: 14
                anchors.verticalCenter: parent.verticalCenter
                size: 42
                textSource: conversationDelegate.initial
                colorBg: conversationDelegate.avatarColor
                textSize: HusAvatar.Size_Auto

                HusBadge {
                    dot: true
                    visible: conversationDelegate.itemKind === "direct"
                    badgeState: conversationDelegate.online
                                ? HusBadge.State_Success
                                : HusBadge.State_Default
                }
            }

            Column {
                anchors.left: conversationAvatar.right
                anchors.right: conversationMeta.left
                anchors.leftMargin: 12
                anchors.rightMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                spacing: 4

                HusText {
                    width: parent.width
                    text: conversationDelegate.title
                    color: HusTheme.Primary.colorTextBase
                    elide: Text.ElideRight
                    font.pixelSize: HusTheme.Primary.fontPrimarySize
                    font.weight: Font.Medium
                }

                HusText {
                    width: parent.width
                    text: root.contactsMode
                          ? conversationDelegate.statusText
                          : conversationDelegate.lastMessage
                    color: HusTheme.Primary.colorTextTertiary
                    elide: Text.ElideRight
                    font.pixelSize: HusTheme.Primary.fontPrimarySize
                    font.weight: conversationDelegate.unread > 0
                                 ? Font.Medium
                                 : Font.Normal
                }
            }

            Column {
                id: conversationMeta

                anchors.right: parent.right
                anchors.rightMargin: 14
                anchors.verticalCenter: parent.verticalCenter
                width: 54
                spacing: 7

                HusText {
                    width: parent.width
                    text: conversationDelegate.lastTime
                    color: HusTheme.Primary.colorTextQuaternary
                    horizontalAlignment: Text.AlignRight
                    font.pixelSize: Math.max(11, HusTheme.Primary.fontPrimarySize - 1)
                    font.weight: conversationDelegate.unread > 0
                                 ? Font.Medium
                                 : Font.Normal
                }

                Item {
                    width: parent.width
                    height: 20

                    HusBadge {
                        anchors.right: parent.right
                        count: conversationDelegate.unread
                    }
                }
            }

            MouseArea {
                id: conversationMouse

                anchors.fill: parent
                activeFocusOnTab: true
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                Accessible.role: Accessible.ListItem
                Accessible.name: qsTr("%1，%2")
                                     .arg(conversationDelegate.title)
                                     .arg(conversationDelegate.statusText)
                Accessible.description: conversationDelegate.unread > 0
                                        ? qsTr("%1 条未读消息")
                                              .arg(conversationDelegate.unread)
                                        : qsTr("没有未读消息")
                onClicked: root.conversationSelected(conversationDelegate.itemId)
                Keys.onReturnPressed: root.conversationSelected(conversationDelegate.itemId)
                Keys.onSpacePressed: root.conversationSelected(conversationDelegate.itemId)
            }
        }
    }

    Column {
        anchors.centerIn: conversationList
        width: Math.max(0, conversationList.width - 40)
        spacing: 8
        visible: conversationList.count === 0

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            width: 52
            height: 52
            radius: HusTheme.Primary.radiusPrimary
            color: HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, HusTheme.isDark ? 0.18 : 0.1)
            border.width: 1
            border.color: HusTheme.Primary.colorSplit
            Accessible.ignored: true

            HusIconText {
                anchors.centerIn: parent
                iconSource: root.contactsMode
                            ? root.groupContactsMode
                              ? HusIcon.TeamOutlined
                              : HusIcon.ContactsOutlined
                            : HusIcon.MessageOutlined
                iconSize: 23
                colorIcon: HusTheme.Primary.colorPrimary
            }
        }

        HusText {
            width: parent.width
            text: root.searchKeyword.length > 0
                  ? qsTr("没有匹配结果")
                  : root.contactsMode
                    ? root.groupContactsMode
                      ? qsTr("还没有群聊")
                      : LanChat.running
                        ? qsTr("正在发现联系人")
                        : qsTr("开始发现联系人")
                    : qsTr("还没有会话")
            color: HusTheme.Primary.colorTextBase
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: HusTheme.Primary.fontPrimarySizeHeading5
            font.weight: Font.Medium
        }

        HusText {
            width: parent.width
            text: root.searchKeyword.length > 0
                  ? qsTr("没有找到与“%1”匹配的内容").arg(root.searchKeyword)
                  : root.contactsMode
                    ? root.groupContactsMode
                      ? qsTr("点击上方按钮创建群聊")
                      : LanChat.running
                        ? qsTr("正在自动发现同一局域网内的联系人…")
                        : LanChat.lastError.length > 0
                          ? LanChat.lastError
                          : qsTr("点击左侧刷新好友按钮启动局域网发现")
                    : qsTr("从联系人页开始单聊，或点击上方按钮创建群聊")
            color: HusTheme.Primary.colorTextTertiary
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            font.pixelSize: HusTheme.Primary.fontPrimarySize
        }
    }
}
