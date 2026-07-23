pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import HuskarUI.Basic

Item {
    id: root

    property string errorText: ""

    signal closeRequested()
    signal saved()

    function loadSettings(): void {
        themeModeSelect.currentIndex = themeModeSelect.indexOfValue(AppSettings.themeMode);
        primaryColorInput.text = AppSettings.primaryColor;
        animationsSwitch.checked = AppSettings.animationsEnabled;
        notificationsSwitch.checked = AppSettings.notificationsEnabled;
        logLevelSelect.currentIndex = logLevelSelect.indexOfValue(AppSettings.logLevel);
        root.errorText = "";
    }

    function saveSettings(): void {
        if (AppSettings.save(themeModeSelect.currentValue,
                             primaryColorInput.text,
                             animationsSwitch.checked,
                             notificationsSwitch.checked,
                             logLevelSelect.currentValue)) {
            root.errorText = "";
            root.saved();
        } else {
            root.errorText = AppSettings.lastError;
        }
    }

    Component.onCompleted: root.loadSettings()

    Flickable {
        id: settingsFlickable

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: footer.top
        contentWidth: width
        contentHeight: settingsColumn.implicitHeight + 48
        boundsBehavior: Flickable.StopAtBounds
        clip: true
        ScrollBar.vertical: HusScrollBar { }

        ColumnLayout {
            id: settingsColumn

            x: 24
            y: 24
            width: settingsFlickable.width - 48
            spacing: 16

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 278
                radius: 12
                color: HusTheme.Primary.colorBgContainer
                border.width: 1
                border.color: HusTheme.Primary.colorBorderSecondary

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 16

                    HusText {
                        Layout.fillWidth: true
                        text: qsTr("外观与体验")
                        color: HusTheme.Primary.colorTextBase
                        font.pixelSize: HusTheme.Primary.fontPrimarySizeHeading4
                        font.weight: Font.Medium
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 16

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            HusText {
                                Layout.fillWidth: true
                                text: qsTr("主题模式")
                                color: HusTheme.Primary.colorTextBase
                                font.pixelSize: HusTheme.Primary.fontPrimarySize
                                font.weight: Font.Medium
                            }

                            HusText {
                                Layout.fillWidth: true
                                text: qsTr("选择浅色、深色或跟随系统")
                                color: HusTheme.Primary.colorTextTertiary
                                font.pixelSize: HusTheme.Primary.fontPrimarySize
                            }
                        }

                        HusSelect {
                            id: themeModeSelect

                            Layout.preferredWidth: 150
                            Layout.preferredHeight: 40
                            clearEnabled: false
                            contentDescription: qsTr("主题模式")
                            model: [
                                { "label": qsTr("跟随系统"), "value": "system" },
                                { "label": qsTr("浅色"), "value": "light" },
                                { "label": qsTr("深色"), "value": "dark" }
                            ]
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 16

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            HusText {
                                Layout.fillWidth: true
                                text: qsTr("主题色")
                                color: HusTheme.Primary.colorTextBase
                                font.pixelSize: HusTheme.Primary.fontPrimarySize
                                font.weight: Font.Medium
                            }

                            HusText {
                                Layout.fillWidth: true
                                text: qsTr("使用六位十六进制颜色值")
                                color: HusTheme.Primary.colorTextTertiary
                                font.pixelSize: HusTheme.Primary.fontPrimarySize
                            }
                        }

                        Rectangle {
                            Layout.preferredWidth: 28
                            Layout.preferredHeight: 28
                            radius: 8
                            color: primaryColorInput.acceptableInput
                                   ? primaryColorInput.text
                                   : "transparent"
                            border.width: 1
                            border.color: HusTheme.Primary.colorBorder
                            Accessible.ignored: true
                        }

                        HusInput {
                            id: primaryColorInput

                            Layout.preferredWidth: 120
                            Layout.preferredHeight: 40
                            maximumLength: 7
                            selectByMouse: true
                            horizontalAlignment: Text.AlignHCenter
                            contentDescription: qsTr("主题色")
                            validator: RegularExpressionValidator {
                                regularExpression: /^#[0-9A-Fa-f]{6}$/
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 16

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            HusText {
                                Layout.fillWidth: true
                                text: qsTr("界面动画")
                                color: HusTheme.Primary.colorTextBase
                                font.pixelSize: HusTheme.Primary.fontPrimarySize
                                font.weight: Font.Medium
                            }

                            HusText {
                                Layout.fillWidth: true
                                text: qsTr("关闭后界面切换和控件反馈将立即完成")
                                color: HusTheme.Primary.colorTextTertiary
                                font.pixelSize: HusTheme.Primary.fontPrimarySize
                            }
                        }

                        HusSwitch {
                            id: animationsSwitch

                            Layout.preferredWidth: 48
                            Layout.preferredHeight: 28
                            contentDescription: qsTr("启用界面动画")
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 136
                radius: 12
                color: HusTheme.Primary.colorBgContainer
                border.width: 1
                border.color: HusTheme.Primary.colorBorderSecondary

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 15

                    HusText {
                        Layout.fillWidth: true
                        text: qsTr("消息通知")
                        color: HusTheme.Primary.colorTextBase
                        font.pixelSize: HusTheme.Primary.fontPrimarySizeHeading4
                        font.weight: Font.Medium
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 16

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            HusText {
                                Layout.fillWidth: true
                                text: qsTr("系统通知")
                                color: HusTheme.Primary.colorTextBase
                                font.pixelSize: HusTheme.Primary.fontPrimarySize
                                font.weight: Font.Medium
                            }

                            HusText {
                                Layout.fillWidth: true
                                text: qsTr("窗口不在前台时提示新消息和文件接收结果")
                                color: HusTheme.Primary.colorTextTertiary
                                wrapMode: Text.Wrap
                                font.pixelSize: HusTheme.Primary.fontPrimarySize
                            }
                        }

                        HusSwitch {
                            id: notificationsSwitch

                            Layout.preferredWidth: 48
                            Layout.preferredHeight: 28
                            contentDescription: qsTr("启用系统通知")
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 136
                radius: 12
                color: HusTheme.Primary.colorBgContainer
                border.width: 1
                border.color: HusTheme.Primary.colorBorderSecondary

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 15

                    HusText {
                        Layout.fillWidth: true
                        text: qsTr("诊断日志")
                        color: HusTheme.Primary.colorTextBase
                        font.pixelSize: HusTheme.Primary.fontPrimarySizeHeading4
                        font.weight: Font.Medium
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 16

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            HusText {
                                Layout.fillWidth: true
                                text: qsTr("日志级别")
                                color: HusTheme.Primary.colorTextBase
                                font.pixelSize: HusTheme.Primary.fontPrimarySize
                                font.weight: Font.Medium
                            }

                            HusText {
                                Layout.fillWidth: true
                                text: qsTr("调试问题时可临时切换为 Debug 或 Trace")
                                color: HusTheme.Primary.colorTextTertiary
                                font.pixelSize: HusTheme.Primary.fontPrimarySize
                            }
                        }

                        HusSelect {
                            id: logLevelSelect

                            Layout.preferredWidth: 150
                            Layout.preferredHeight: 40
                            clearEnabled: false
                            contentDescription: qsTr("日志级别")
                            model: [
                                { "label": qsTr("Trace"), "value": "trace" },
                                { "label": qsTr("Debug"), "value": "debug" },
                                { "label": qsTr("Info"), "value": "info" },
                                { "label": qsTr("Warning"), "value": "warn" },
                                { "label": qsTr("Error"), "value": "error" },
                                { "label": qsTr("Critical"), "value": "critical" },
                                { "label": qsTr("关闭"), "value": "off" }
                            ]
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 108
                radius: 12
                color: HusTheme.Primary.colorFillQuaternary
                border.width: 1
                border.color: HusTheme.Primary.colorBorderSecondary

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 16

                    HusIconText {
                        Layout.preferredWidth: 28
                        iconSource: HusIcon.InfoCircleOutlined
                        iconSize: 24
                        colorIcon: HusTheme.Primary.colorPrimary
                        Accessible.ignored: true
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 5

                        HusText {
                            Layout.fillWidth: true
                            text: qsTr("关于 YueLink")
                            color: HusTheme.Primary.colorTextBase
                            font.pixelSize: HusTheme.Primary.fontPrimarySize
                            font.weight: Font.Medium
                        }

                        HusText {
                            Layout.fillWidth: true
                            text: qsTr("局域网即时聊天与文件传输工具 · 当前版本 0.1")
                            color: HusTheme.Primary.colorTextSecondary
                            wrapMode: Text.Wrap
                            font.pixelSize: HusTheme.Primary.fontPrimarySize
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        id: footer

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 72
        color: HusTheme.Primary.colorBgContainer
        border.width: 1
        border.color: HusTheme.Primary.colorBorderSecondary

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 24
            anchors.rightMargin: 24
            spacing: 10

            HusText {
                Layout.fillWidth: true
                visible: root.errorText.length > 0
                text: root.errorText
                color: HusTheme.Primary.colorError
                elide: Text.ElideRight
                font.pixelSize: HusTheme.Primary.fontPrimarySize
            }

            HusButton {
                Layout.preferredWidth: 92
                Layout.preferredHeight: 40
                text: qsTr("取消")
                contentDescription: qsTr("取消设置修改")
                onClicked: root.closeRequested()
            }

            HusButton {
                Layout.preferredWidth: 108
                Layout.preferredHeight: 40
                type: HusButton.Type_Primary
                text: qsTr("保存设置")
                enabled: primaryColorInput.acceptableInput
                         && themeModeSelect.currentIndex >= 0
                         && logLevelSelect.currentIndex >= 0
                contentDescription: qsTr("保存应用设置")
                onClicked: root.saveSettings()
            }
        }
    }
}
