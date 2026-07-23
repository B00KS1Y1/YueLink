pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import HuskarUI.Basic

Item {
    id: root

    property string selectedPeerId: ""
    readonly property string searchKeyword: searchInput.text.trim().toLocaleLowerCase()

    signal friendSelected(string peerId)

    Item {
        id: searchSection

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 54

        RowLayout {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            spacing: 8

            HusInput {
                id: searchInput

                Layout.fillWidth: true
                Layout.preferredHeight: 38
                iconSource: HusIcon.SearchOutlined
                iconPosition: HusInput.Position_Left
                clearEnabled: "active"
                type: HusInput.Type_Filled
                placeholderText: qsTr("搜索好友")
                contentDescription: qsTr("搜索好友")
            }

            HusIconButton {
                Layout.preferredWidth: 38
                Layout.preferredHeight: 38
                padding: 0
                type: HusButton.Type_Filled
                iconSource: HusIcon.PlusOutlined
                iconSize: 18
                contentDescription: qsTr("添加好友")
            }
        }
    }

    Item {
        id: listHeader

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: searchSection.bottom
        height: 42

        HusText {
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.verticalCenter: parent.verticalCenter
            text: qsTr("局域网好友")
            color: HusTheme.Primary.colorTextBase
            font.pixelSize: HusTheme.Primary.fontPrimarySize
            font.weight: Font.Medium
        }

        HusText {
            anchors.right: parent.right
            anchors.rightMargin: 20
            anchors.verticalCenter: parent.verticalCenter
            text: LanChat.totalUnreadCount > 0
                  ? qsTr("%1 条未读 · %2 人在线")
                        .arg(LanChat.totalUnreadCount)
                        .arg(LanChat.onlineCount)
                  : qsTr("%1 人在线").arg(LanChat.onlineCount)
            color: HusTheme.Primary.colorTextTertiary
            font.pixelSize: HusTheme.Primary.fontPrimarySize
        }
    }

    ListView {
        id: friendList

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: listHeader.bottom
        anchors.bottom: parent.bottom
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        anchors.bottomMargin: 10
        model: LanChat.peers
        currentIndex: -1
        boundsBehavior: Flickable.StopAtBounds
        reuseItems: true
        clip: true
        ScrollBar.vertical: HusScrollBar { }

        delegate: Item {
            id: friendDelegate

            required property int index
            required property string peerId
            required property string friendName
            required property string initial
            required property string statusText
            required property string lastMessage
            required property string lastTime
            required property color avatarColor
            required property bool online
            required property int unread
            readonly property bool matchesSearch: root.searchKeyword.length === 0 ||
                                                      friendName.toLocaleLowerCase().indexOf(root.searchKeyword) !== -1

            width: friendList.width
            height: matchesSearch ? 72 : 0
            visible: matchesSearch

            Rectangle {
                anchors.fill: parent
                anchors.margins: 2
                radius: 8
                color: root.selectedPeerId === friendDelegate.peerId
                       ? HusTheme.Primary.colorPrimaryBg
                       : friendMouse.containsMouse
                         ? HusTheme.Primary.colorFillTertiary
                         : "transparent"
                border.width: friendMouse.activeFocus ? 1 : 0
                border.color: HusTheme.Primary.colorPrimary
            }

            HusAvatar {
                id: friendAvatar

                anchors.left: parent.left
                anchors.leftMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                size: 44
                textSource: friendDelegate.initial
                colorBg: friendDelegate.avatarColor
                textSize: HusAvatar.Size_Auto

                HusBadge {
                    dot: true
                    badgeState: friendDelegate.online
                                ? HusBadge.State_Success
                                : HusBadge.State_Default
                }
            }

            Column {
                anchors.left: friendAvatar.right
                anchors.right: friendMeta.left
                anchors.leftMargin: 12
                anchors.rightMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                spacing: 6

                HusText {
                    width: parent.width
                    text: friendDelegate.friendName
                    color: HusTheme.Primary.colorTextBase
                    elide: Text.ElideRight
                    font.pixelSize: HusTheme.Primary.fontPrimarySize
                    font.weight: Font.Medium
                }

                HusText {
                    width: parent.width
                    text: friendDelegate.lastMessage
                    color: HusTheme.Primary.colorTextTertiary
                    elide: Text.ElideRight
                    font.pixelSize: HusTheme.Primary.fontPrimarySize
                    font.weight: friendDelegate.unread > 0
                                 ? Font.Medium
                                 : Font.Normal
                }
            }

            Column {
                id: friendMeta

                anchors.right: parent.right
                anchors.rightMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                width: 54
                spacing: 7

                HusText {
                    width: parent.width
                    text: friendDelegate.lastTime
                    color: HusTheme.Primary.colorTextTertiary
                    horizontalAlignment: Text.AlignRight
                    font.pixelSize: HusTheme.Primary.fontPrimarySize
                    font.weight: friendDelegate.unread > 0
                                 ? Font.Medium
                                 : Font.Normal
                }

                Item {
                    width: parent.width
                    height: 20

                    HusBadge {
                        anchors.right: parent.right
                        count: friendDelegate.unread
                    }
                }
            }

            MouseArea {
                id: friendMouse

                anchors.fill: parent
                activeFocusOnTab: true
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                Accessible.role: Accessible.ListItem
                Accessible.name: qsTr("%1，%2").arg(friendDelegate.friendName)
                                                .arg(friendDelegate.statusText)
                Accessible.description: friendDelegate.unread > 0
                                        ? qsTr("%1 条未读消息")
                                              .arg(friendDelegate.unread)
                                        : qsTr("没有未读消息")
                onClicked: root.friendSelected(friendDelegate.peerId)
                Keys.onReturnPressed: root.friendSelected(friendDelegate.peerId)
                Keys.onSpacePressed: root.friendSelected(friendDelegate.peerId)
            }
        }
    }

    HusText {
        anchors.centerIn: friendList
        width: Math.max(0, friendList.width - 40)
        visible: friendList.count === 0
        text: LanChat.running
              ? qsTr("正在发现同一局域网内的好友…")
              : LanChat.lastError.length > 0
                ? LanChat.lastError
                : qsTr("局域网服务未启动")
        color: HusTheme.Primary.colorTextTertiary
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.Wrap
        font.pixelSize: HusTheme.Primary.fontPrimarySize
    }
}
