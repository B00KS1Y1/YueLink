pragma ComponentBehavior: Bound

import QtQuick
import HuskarUI.Basic

Item {
    id: root

    implicitWidth: 180
    implicitHeight: 46

    signal profileEditRequested()

    Row {
        anchors.fill: parent
        spacing: 8

        Item {
            width: 40
            height: parent.height

            HusAvatar {
                id: localAvatar

                anchors.centerIn: parent
                size: 34
                textSource: LanChat.localInitial
                imageSource: LanChat.localAvatarUrl.toString()
                colorBg: LanChat.localAvatarColor
                textSize: HusAvatar.Size_Auto

                Rectangle {
                    anchors.fill: parent
                    radius: width * 0.5
                    color: "transparent"
                    border.width: 1
                    border.color: HusTheme.Primary.colorPrimary
                    Accessible.ignored: true
                }

                HusBadge {
                    dot: true
                    badgeState: LanChat.running
                                ? HusBadge.State_Success
                                : HusBadge.State_Default
                }
            }

            MouseArea {
                anchors.fill: parent
                activeFocusOnTab: true
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                Accessible.role: Accessible.Button
                Accessible.name: qsTr("编辑个人资料")
                onClicked: root.profileEditRequested()
                Keys.onReturnPressed: root.profileEditRequested()
                Keys.onSpacePressed: root.profileEditRequested()
            }
        }

        Column {
            width: 132
            anchors.verticalCenter: parent.verticalCenter
            spacing: 1

            HusText {
                width: parent.width
                text: LanChat.localName
                color: HusTheme.Primary.colorTextBase
                elide: Text.ElideRight
                font.pixelSize: HusTheme.Primary.fontPrimarySize
                font.weight: Font.Medium
            }

            HusText {
                width: parent.width
                text: LanChat.running
                      ? qsTr("在线 · YueLink")
                      : qsTr("网络服务未启动")
                color: HusTheme.Primary.colorTextTertiary
                elide: Text.ElideRight
                font.pixelSize: Math.max(11, HusTheme.Primary.fontPrimarySize - 2)
            }
        }
    }
}
