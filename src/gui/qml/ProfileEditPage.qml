pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic

Item {
    id: root

    property string errorText: ""

    signal closeRequested()
    signal saved()

    function saveProfile(): void {
        if (LanChat.updateLocalProfile(displayNameInput.text)) {
            root.errorText = "";
            root.saved();
        } else {
            root.errorText = LanChat.lastError;
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 28
        spacing: 20

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 132
            radius: 12
            color: HusTheme.Primary.colorFillQuaternary
            border.width: 1
            border.color: HusTheme.Primary.colorBorderSecondary

            RowLayout {
                anchors.fill: parent
                anchors.margins: 22
                spacing: 18

                HusAvatar {
                    Layout.preferredWidth: 76
                    Layout.preferredHeight: 76
                    size: 76
                    textSource: displayNameInput.text.trim().length > 0
                                ? displayNameInput.text.trim().slice(0, 1).toUpperCase()
                                : "?"
                    colorBg: AppSettings.primaryColor
                    textSize: HusAvatar.Size_Auto
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 7

                    HusText {
                        Layout.fillWidth: true
                        text: displayNameInput.text.trim().length > 0
                              ? displayNameInput.text.trim()
                              : qsTr("你的昵称")
                        color: HusTheme.Primary.colorTextBase
                        elide: Text.ElideRight
                        font.pixelSize: HusTheme.Primary.fontPrimarySizeHeading4
                        font.weight: Font.Medium
                    }

                    HusText {
                        Layout.fillWidth: true
                        text: LanChat.running
                              ? qsTr("在线 · 局域网好友可以看到此昵称")
                              : qsTr("网络服务启动后好友可以看到此昵称")
                        color: HusTheme.Primary.colorTextSecondary
                        wrapMode: Text.Wrap
                        font.pixelSize: HusTheme.Primary.fontPrimarySize
                    }
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 9

            HusText {
                Layout.fillWidth: true
                text: qsTr("昵称")
                color: HusTheme.Primary.colorTextBase
                font.pixelSize: HusTheme.Primary.fontPrimarySize
                font.weight: Font.Medium
            }

            HusInput {
                id: displayNameInput

                Layout.fillWidth: true
                Layout.preferredHeight: 34
                text: LanChat.localName
                maximumLength: 64
                clearEnabled: "active"
                iconSource: HusIcon.UserOutlined
                iconSize: 16
                iconPosition: HusInput.Position_Left
                verticalAlignment: TextInput.AlignVCenter
                placeholderText: qsTr("输入昵称")
                contentDescription: qsTr("昵称")
                selectByMouse: true
                onAccepted: root.saveProfile()
            }

            HusText {
                Layout.fillWidth: true
                text: qsTr("%1/64 个字符").arg(displayNameInput.text.length)
                color: HusTheme.Primary.colorTextTertiary
                horizontalAlignment: Text.AlignRight
                font.pixelSize: HusTheme.Primary.fontPrimarySize
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 76
            radius: 10
            color: HusTheme.Primary.colorPrimaryBg

            RowLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 12

                HusIconText {
                    Layout.preferredWidth: 24
                    iconSource: HusIcon.InfoCircleOutlined
                    iconSize: 21
                    colorIcon: HusTheme.Primary.colorPrimary
                    Accessible.ignored: true
                }

                HusText {
                    Layout.fillWidth: true
                    text: qsTr("保存后会立即向局域网好友广播新的昵称，不会中断当前聊天。")
                    color: HusTheme.Primary.colorTextSecondary
                    wrapMode: Text.Wrap
                    font.pixelSize: HusTheme.Primary.fontPrimarySize
                }
            }
        }

        HusText {
            Layout.fillWidth: true
            visible: root.errorText.length > 0
            text: root.errorText
            color: HusTheme.Primary.colorError
            wrapMode: Text.Wrap
            font.pixelSize: HusTheme.Primary.fontPrimarySize
        }

        Item {
            Layout.fillHeight: true
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Item {
                Layout.fillWidth: true
            }

            HusButton {
                Layout.preferredWidth: 92
                Layout.preferredHeight: 40
                text: qsTr("取消")
                contentDescription: qsTr("取消编辑个人信息")
                onClicked: root.closeRequested()
            }

            HusButton {
                Layout.preferredWidth: 108
                Layout.preferredHeight: 40
                type: HusButton.Type_Primary
                text: qsTr("保存修改")
                enabled: displayNameInput.text.trim().length > 0
                contentDescription: qsTr("保存个人信息")
                onClicked: root.saveProfile()
            }
        }
    }
}
