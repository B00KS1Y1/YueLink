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
    property color controlSurfaceColor: HusThemeFunctions.alpha(
                                            HusTheme.Primary.colorBgBase, 0.42)
    readonly property bool groupContactsMode: contactsMode
                                               && contactTypeSelector.currentIndex === 1
    readonly property string searchKeyword: (contactsMode
                                              ? groupContactsMode
                                                ? LanChat.groupSearchText
                                                : LanChat.peerSearchText
                                              : LanChat.conversationSearchText).trim()
    property string contextConversationId: ""
    property string contextConversationTitle: ""
    property string contextConversationKind: ""
    property bool contextConversationPinned: false
    property int contextConversationUnread: 0

    signal conversationSelected(string conversationId)
    signal createGroupRequested()

    function openConversationMenu(delegateItem, localX: real, localY: real,
                                  conversationId: string, title: string,
                                  kind: string, pinned: bool, unread: int): void {
        const position = delegateItem.mapToItem(root, localX, localY);
        contextConversationId = conversationId;
        contextConversationTitle = title;
        contextConversationKind = kind;
        contextConversationPinned = pinned;
        contextConversationUnread = unread;
        conversationContextMenu.x = position.x;
        conversationContextMenu.y = position.y;
        conversationContextMenu.open();
    }

    Item {
        id: searchSection

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 48

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
                Layout.preferredHeight: 32
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

                Layout.preferredWidth: 32
                Layout.preferredHeight: 32
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
            colorBg: root.controlSurfaceColor
            colorIndicatorBg: HusThemeFunctions.alpha(
                                  HusTheme.Primary.colorTextBase,
                                  HusTheme.isDark ? 0.14 : 0.1)
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

    ListView {
        id: conversationList

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: contactTypeSection.bottom
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
            required property bool pinned
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
                       : conversationContextMenu.visible
                         && root.contextConversationId === conversationDelegate.itemId
                         ? HusTheme.Primary.colorFillSecondary
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

                    RowLayout {
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 6

                        HusIconText {
                            Layout.alignment: Qt.AlignVCenter
                            visible: !root.contactsMode && conversationDelegate.pinned
                            iconSource: HusIcon.PushpinFilled
                            iconSize: 12
                            colorIcon: HusTheme.Primary.colorTextQuaternary
                            Accessible.name: qsTr("已置顶")
                        }

                        HusBadge {
                            Layout.alignment: Qt.AlignVCenter
                            visible: conversationDelegate.unread > 0
                            count: conversationDelegate.unread
                        }
                    }
                }
            }

            MouseArea {
                id: conversationMouse

                anchors.fill: parent
                activeFocusOnTab: true
                acceptedButtons: root.contactsMode
                                 ? Qt.LeftButton
                                 : Qt.LeftButton | Qt.RightButton
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
                onClicked: mouse => {
                    if (mouse.button === Qt.RightButton) {
                        root.openConversationMenu(
                                    conversationDelegate,
                                    mouse.x,
                                    mouse.y,
                                    conversationDelegate.itemId,
                                    conversationDelegate.title,
                                    conversationDelegate.itemKind,
                                    conversationDelegate.pinned,
                                    conversationDelegate.unread);
                        return;
                    }
                    root.conversationSelected(conversationDelegate.itemId);
                }
                Keys.onReturnPressed: root.conversationSelected(conversationDelegate.itemId)
                Keys.onSpacePressed: root.conversationSelected(conversationDelegate.itemId)
                Keys.onPressed: event => {
                    const contextKey = event.key === Qt.Key_Menu
                            || (event.key === Qt.Key_F10
                                && (event.modifiers & Qt.ShiftModifier));
                    if (!root.contactsMode && contextKey) {
                        root.openConversationMenu(
                                    conversationDelegate,
                                    conversationDelegate.width - 18,
                                    conversationDelegate.height * 0.5,
                                    conversationDelegate.itemId,
                                    conversationDelegate.title,
                                    conversationDelegate.itemKind,
                                    conversationDelegate.pinned,
                                    conversationDelegate.unread);
                        event.accepted = true;
                    }
                }
            }
        }
    }

    HusContextMenu {
        id: conversationContextMenu

        defaultMenuWidth: 164
        initModel: [
            {
                key: "pin",
                label: root.contextConversationPinned
                       ? qsTr("取消置顶")
                       : qsTr("置顶会话"),
                iconSource: root.contextConversationPinned
                            ? HusIcon.PushpinFilled
                            : HusIcon.PushpinOutlined
            },
            {
                key: "markRead",
                label: qsTr("标为已读"),
                iconSource: HusIcon.CheckOutlined,
                enabled: root.contextConversationUnread > 0
            },
            { type: "divider" },
            {
                key: "delete",
                label: qsTr("删除会话"),
                iconSource: HusIcon.DeleteOutlined
            }
        ]
        onClickMenu: (deep, key) => {
            if (deep !== 0 || root.contextConversationId.length === 0)
                return;
            if (key === "pin") {
                const targetPinned = !root.contextConversationPinned;
                if (LanChat.setConversationPinned(root.contextConversationId,
                                                  targetPinned))
                    root.contextConversationPinned = targetPinned;
            } else if (key === "markRead") {
                if (LanChat.markConversationRead(root.contextConversationId))
                    root.contextConversationUnread = 0;
            } else if (key === "delete") {
                deleteConversationModal.conversationId = root.contextConversationId;
                deleteConversationModal.conversationTitle = root.contextConversationTitle;
                deleteConversationModal.conversationKind = root.contextConversationKind;
                deleteConversationModal.open();
            }
        }
    }

    HusModal {
        id: deleteConversationModal

        property string conversationId: ""
        property string conversationTitle: ""
        property string conversationKind: ""

        width: 440
        title: qsTr("删除会话")
        iconSource: HusIcon.DeleteOutlined
        colorIcon: HusTheme.Primary.colorError
        confirmText: qsTr("删除")
        cancelText: qsTr("取消")
        maskClosable: true
        bodyDelegate: Column {
            height: implicitHeight
            spacing: 8

            HusText {
                width: parent.width
                text: deleteConversationModal.conversationKind === "group"
                      ? qsTr("确定删除群聊“%1”的本地会话吗？消息记录会从本机删除，但不会退出群聊或通知其他成员。").arg(deleteConversationModal.conversationTitle)
                      : qsTr("确定删除与“%1”的本地会话吗？消息记录会从本机删除，但联系人仍会保留。").arg(deleteConversationModal.conversationTitle)
                color: HusTheme.Primary.colorTextTertiary
                font.pixelSize: HusTheme.Primary.fontPrimarySize
                wrapMode: Text.WordWrap
            }

            HusText {
                width: parent.width
                text: qsTr("此操作无法撤销。已保存的本地文件不会被删除；收到新消息或从联系人页重新打开后，会话会再次出现。")
                color: HusTheme.Primary.colorTextQuaternary
                font.pixelSize: Math.max(12, HusTheme.Primary.fontPrimarySize - 1)
                wrapMode: Text.WordWrap
            }
        }
        onConfirm: {
            if (LanChat.removeConversation(conversationId))
                close();
        }
        onCancel: close()
        onClosed: {
            conversationId = "";
            conversationTitle = "";
            conversationKind = "";
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
