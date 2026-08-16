pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Dialogs
import QtQuick.Layouts
import HuskarUI.Basic

Item {
    id: root

    property string errorText: ""
    property url avatarUrl: LanChat.localAvatarUrl

    signal closeRequested()
    signal saved()

    function saveProfile(): void {
        if (LanChat.updateLocalProfile(displayNameInput.text,
                                       root.avatarUrl,
                                       avatarColorPicker.toHexString(avatarColorPicker.value))) {
            root.errorText = "";
            root.saved();
        } else {
            root.errorText = LanChat.lastError;
        }
    }

    FileDialog {
        id: avatarFileDialog

        title: qsTr("选择头像图片")
        fileMode: FileDialog.OpenFile
        nameFilters: [qsTr("图片文件 (*.png *.jpg *.jpeg *.bmp *.webp)"), qsTr("所有文件 (*)")]
        onAccepted: root.avatarUrl = selectedFile
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 14

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 116
            radius: HusTheme.Primary.radiusPrimary
            color: HusTheme.HusCard.colorBg
            border.width: 1
            border.color: HusTheme.Primary.colorSplit

            RowLayout {
                anchors.fill: parent
                anchors.margins: 18
                spacing: 18

                AppAvatar {
                    Layout.preferredWidth: 76
                    Layout.preferredHeight: 76
                    size: 76
                    imageSource: root.avatarUrl
                    textSource: displayNameInput.text.trim().length > 0
                                ? displayNameInput.text.trim().slice(0, 1).toUpperCase()
                                : "?"
                    colorBg: avatarColorPicker.toHexString(avatarColorPicker.value)
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
                        color: HusTheme.Primary.colorTextTertiary
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
                color: HusTheme.Primary.colorTextQuaternary
                horizontalAlignment: Text.AlignRight
                font.pixelSize: HusTheme.Primary.fontPrimarySize
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 16

            HusText {
                Layout.fillWidth: true
                text: qsTr("头像颜色")
                color: HusTheme.Primary.colorTextBase
                font.pixelSize: HusTheme.Primary.fontPrimarySize
                font.weight: Font.Medium
            }

            HusColorPicker {
                id: avatarColorPicker

                Layout.preferredWidth: 160
                Layout.preferredHeight: 34
                defaultValue: LanChat.localAvatarColor
                showText: true
                alphaEnabled: false
                format: "hex"
                title: qsTr("选择头像颜色")
                Accessible.name: qsTr("头像颜色")
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            HusText {
                Layout.fillWidth: true
                text: root.avatarUrl.toString().length > 0
                      ? qsTr("已选择本地头像图片")
                      : qsTr("未选择图片，将显示昵称首字母")
                color: HusTheme.Primary.colorTextTertiary
                elide: Text.ElideRight
                font.pixelSize: HusTheme.Primary.fontPrimarySize
            }

            HusIconButton {
                Layout.preferredWidth: 86
                Layout.preferredHeight: 34
                text: qsTr("选择")
                iconSource: HusIcon.PictureOutlined
                iconSize: 16
                contentDescription: qsTr("选择本地头像图片")
                onClicked: avatarFileDialog.open()
            }

            HusIconButton {
                Layout.preferredWidth: 72
                Layout.preferredHeight: 34
                visible: root.avatarUrl.toString().length > 0
                text: qsTr("清除")
                iconSource: HusIcon.DeleteOutlined
                iconSize: 16
                contentDescription: qsTr("清除头像图片")
                onClicked: root.avatarUrl = ""
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 76
            radius: HusTheme.Primary.radiusPrimary
            color: HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, HusTheme.isDark ? 0.18 : 0.1)

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
                    text: qsTr("昵称变更会立即广播给局域网好友；头像图片和颜色仅保存在当前设备。")
                    color: HusTheme.Primary.colorTextTertiary
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
