import QtQuick
import HuskarUI.Basic

Item {
    id: root

    signal profileEditRequested()
    signal settingsRequested()

    implicitHeight: 92

    Rectangle {
        anchors.left: parent.left
        anchors.right: settingsButton.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 8
        radius: 10
        color: profileMouse.containsMouse
               ? HusTheme.Primary.colorFillTertiary
               : "transparent"
        border.width: profileMouse.activeFocus ? 1 : 0
        border.color: HusTheme.Primary.colorPrimary
    }

    Row {
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.verticalCenter: parent.verticalCenter
        spacing: 12

        HusAvatar {
            size: 52
            textSource: LanChat.localInitial
            colorBg: "#4F7CFF"
            textSize: HusAvatar.Size_Auto

            HusBadge {
                dot: true
                badgeState: LanChat.running
                            ? HusBadge.State_Success
                            : HusBadge.State_Default
            }
        }

        Column {
            anchors.verticalCenter: parent.verticalCenter
            width: Math.max(120, root.width - 152)
            spacing: 5

            Row {
                width: parent.width
                spacing: 6

                HusText {
                    width: Math.max(0, parent.width - editIcon.width - parent.spacing)
                    text: LanChat.localName
                    color: HusTheme.Primary.colorTextBase
                    elide: Text.ElideRight
                    font.pixelSize: HusTheme.Primary.fontPrimarySizeHeading5
                    font.weight: Font.Medium
                }

                HusIconText {
                    id: editIcon

                    anchors.verticalCenter: parent.verticalCenter
                    iconSource: HusIcon.EditOutlined
                    iconSize: 14
                    colorIcon: HusTheme.Primary.colorTextTertiary
                    Accessible.ignored: true
                }
            }

            Row {
                width: parent.width
                spacing: 6

                HusBadge {
                    anchors.verticalCenter: parent.verticalCenter
                    dot: true
                    badgeState: LanChat.running
                                ? HusBadge.State_Success
                                : HusBadge.State_Default
                }

                HusText {
                    width: Math.max(0, parent.width - 14)
                    text: LanChat.running
                          ? qsTr("在线 · YueLink")
                          : qsTr("网络服务未启动")
                    color: HusTheme.Primary.colorTextSecondary
                    elide: Text.ElideRight
                    font.pixelSize: HusTheme.Primary.fontPrimarySize
                }
            }
        }
    }

    MouseArea {
        id: profileMouse

        anchors.left: parent.left
        anchors.right: settingsButton.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        activeFocusOnTab: true
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        Accessible.role: Accessible.Button
        Accessible.name: qsTr("编辑个人信息")
        onClicked: root.profileEditRequested()
        Keys.onReturnPressed: root.profileEditRequested()
        Keys.onSpacePressed: root.profileEditRequested()
    }

    HusIconButton {
        id: settingsButton

        anchors.right: parent.right
        anchors.rightMargin: 14
        anchors.verticalCenter: parent.verticalCenter
        width: 38
        height: 38
        padding: 0
        type: HusButton.Type_Text
        iconSource: HusIcon.SettingOutlined
        iconSize: 20
        contentDescription: qsTr("打开设置")
        onClicked: root.settingsRequested()
    }
}
