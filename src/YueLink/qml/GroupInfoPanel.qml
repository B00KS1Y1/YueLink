pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic
import HuskarUI.Basic

Item {
    id: root

    property string groupId: ""
    property string groupTitle: qsTr("群聊信息")
    property var members: []
    readonly property int onlinePeerCount: LanChat.onlineCount

    signal closed()

    function refreshMembers(): void {
        members = LanChat.groupMembers(groupId);
    }

    onGroupIdChanged: refreshMembers()
    onOnlinePeerCountChanged: refreshMembers()
    Component.onCompleted: refreshMembers()

    Connections {
        target: LanChat.conversations

        function onModelReset(): void {
            root.refreshMembers();
        }
    }

    HusDrawer {
        id: infoDrawer

        title: root.groupTitle
        drawerSize: Math.min(420, root.width - 80)
        maskClosable: true
        closePosition: HusDrawer.Position_End
        contentDelegate: Item {
            Column {
                id: summary

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.leftMargin: 22
                anchors.rightMargin: 22
                anchors.topMargin: 18
                spacing: 8

                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: 64
                    height: 64
                    radius: 20
                    color: HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary,
                                                   HusTheme.isDark ? 0.2 : 0.12)

                    HusIconText {
                        anchors.centerIn: parent
                        iconSource: HusIcon.TeamOutlined
                        iconSize: 28
                        colorIcon: HusTheme.Primary.colorPrimary
                    }
                }

                HusText {
                    width: parent.width
                    text: root.groupTitle
                    color: HusTheme.Primary.colorTextBase
                    horizontalAlignment: Text.AlignHCenter
                    elide: Text.ElideRight
                    font.pixelSize: HusTheme.Primary.fontPrimarySizeHeading4
                    font.weight: Font.Medium
                }

                HusText {
                    width: parent.width
                    text: qsTr("%1 位成员").arg(root.members.length)
                    color: HusTheme.Primary.colorTextTertiary
                    horizontalAlignment: Text.AlignHCenter
                    font.pixelSize: HusTheme.Primary.fontPrimarySize
                }
            }

            HusText {
                id: memberTitle

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: summary.bottom
                anchors.leftMargin: 22
                anchors.rightMargin: 22
                anchors.topMargin: 24
                text: qsTr("群成员")
                color: HusTheme.Primary.colorTextSecondary
                font.pixelSize: HusTheme.Primary.fontPrimarySize
                font.weight: Font.Medium
            }

            ListView {
                id: memberList

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: memberTitle.bottom
                anchors.bottom: hintText.top
                anchors.leftMargin: 14
                anchors.rightMargin: 14
                anchors.topMargin: 8
                anchors.bottomMargin: 12
                model: root.members
                boundsBehavior: Flickable.StopAtBounds
                clip: true
                ScrollBar.vertical: HusScrollBar { }

                delegate: Item {
                    id: memberDelegate

                    required property var modelData

                    width: memberList.width
                    height: 62

                    HusAvatar {
                        id: memberAvatar

                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        anchors.verticalCenter: parent.verticalCenter
                        size: 40
                        textSource: memberDelegate.modelData.initial
                        colorBg: memberDelegate.modelData.avatarColor
                        textSize: HusAvatar.Size_Auto

                        HusBadge {
                            dot: true
                            badgeState: memberDelegate.modelData.online
                                        ? HusBadge.State_Success
                                        : HusBadge.State_Default
                        }
                    }

                    Column {
                        anchors.left: memberAvatar.right
                        anchors.right: parent.right
                        anchors.leftMargin: 12
                        anchors.rightMargin: 10
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 4

                        HusText {
                            width: parent.width
                            text: memberDelegate.modelData.local
                                  ? qsTr("%1（我）")
                                        .arg(memberDelegate.modelData.displayName)
                                  : memberDelegate.modelData.displayName
                            color: HusTheme.Primary.colorTextBase
                            elide: Text.ElideRight
                            font.pixelSize: HusTheme.Primary.fontPrimarySize
                            font.weight: Font.Medium
                        }

                        HusText {
                            width: parent.width
                            text: memberDelegate.modelData.owner
                                  ? qsTr("群主")
                                  : memberDelegate.modelData.online
                                    ? qsTr("在线")
                                    : qsTr("离线")
                            color: memberDelegate.modelData.owner
                                   ? HusTheme.Primary.colorPrimary
                                   : HusTheme.Primary.colorTextTertiary
                            font.pixelSize: Math.max(11,
                                                     HusTheme.Primary.fontPrimarySize - 1)
                        }
                    }
                }
            }

            HusText {
                id: hintText

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.leftMargin: 22
                anchors.rightMargin: 22
                anchors.bottomMargin: 22
                text: qsTr("开发阶段暂仅支持查看成员，成员管理将在后续版本开放。")
                color: HusTheme.Primary.colorTextTertiary
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap
                font.pixelSize: HusTheme.Primary.fontPrimarySize
            }
        }

        Component.onCompleted: open()
        onClosed: root.closed()
    }

    Shortcut {
        sequence: "Escape"
        onActivated: infoDrawer.close()
    }
}
