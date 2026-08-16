pragma ComponentBehavior: Bound

import QtQuick
import HuskarUI.Basic

Item {
    id: root

    implicitWidth: 176
    implicitHeight: 42

    signal profileEditRequested()

    Row {
        anchors.fill: parent
        spacing: 8

        Item {
            width: 38
            height: parent.height

            HusAvatar {
                id: localAvatar

                anchors.centerIn: parent
                size: 30
                textSource: LanChat.localInitial
                imageSource: LanChat.localAvatarUrl.toString()
                colorBg: LanChat.localAvatarColor
                textSize: HusAvatar.Size_Auto

                Rectangle {
                    anchors.fill: parent
                    radius: width * 0.5
                    color: "transparent"
                    border.width: localProfileMouse.activeFocus ? 2 : 1
                    border.color: HusTheme.Primary.colorPrimary
                    Accessible.ignored: true
                }

                HusBadge {
                    anchors.left: undefined
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.rightMargin: 1
                    anchors.bottomMargin: 1
                    visible: LanChat.running
                    dot: true
                    badgeState: HusBadge.State_Success
                }
            }

            MouseArea {
                id: localProfileMouse

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
            width: 130
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
                visible: !LanChat.running
                text: qsTr("网络服务未启动")
                color: HusTheme.Primary.colorTextQuaternary
                elide: Text.ElideRight
                font.pixelSize: Math.max(11, HusTheme.Primary.fontPrimarySize - 2)
            }
        }
    }
}
