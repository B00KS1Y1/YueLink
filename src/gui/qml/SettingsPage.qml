pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Dialogs
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
        animationsSwitch.checked = AppSettings.animationsEnabled;
        notificationsSwitch.checked = AppSettings.notificationsEnabled;
        logLevelSelect.currentIndex = logLevelSelect.indexOfValue(AppSettings.logLevel);
        logFilePathInput.text = AppSettings.logFilePath;
        root.errorText = "";
    }

    function saveSettings(): void {
        if (AppSettings.save(themeModeSelect.currentValue,
                             primaryColorPicker.toHexString(primaryColorPicker.value),
                             animationsSwitch.checked,
                             notificationsSwitch.checked,
                             logLevelSelect.currentValue,
                             logFilePathInput.text)) {
            root.errorText = "";
            root.saved();
        } else {
            root.errorText = AppSettings.lastError;
        }
    }

    function localFilePath(fileUrl: url): string {
        const fileUrlText = decodeURIComponent(fileUrl.toString());
        if (fileUrlText.startsWith("file:///"))
            return Qt.platform.os === "windows" ? fileUrlText.substring(8) : fileUrlText.substring(7);
        if (fileUrlText.startsWith("file://"))
            return "//" + fileUrlText.substring(7);
        return "";
    }

    Component.onCompleted: root.loadSettings()

    FileDialog {
        id: logFileDialog

        title: qsTr("选择日志文件")
        fileMode: FileDialog.SaveFile
        defaultSuffix: "log"
        nameFilters: [qsTr("日志文件 (*.log)"), qsTr("所有文件 (*)")]
        onAccepted: logFilePathInput.text = root.localFilePath(selectedFile)
    }

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
                Layout.preferredHeight: appearanceLayout.implicitHeight + 40
                radius: 12
                color: HusTheme.Primary.colorBgContainer
                border.width: 1
                border.color: HusTheme.Primary.colorBorderSecondary

                ColumnLayout {
                    id: appearanceLayout

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

                        HusText {
                            Layout.fillWidth: true
                            text: qsTr("主题模式")
                            color: HusTheme.Primary.colorTextBase
                            font.pixelSize: HusTheme.Primary.fontPrimarySize
                            font.weight: Font.Medium
                        }

                        HusSelect {
                            id: themeModeSelect

                            Layout.preferredWidth: 150
                            Layout.preferredHeight: 34
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

                        HusText {
                            Layout.fillWidth: true
                            text: qsTr("主题色")
                            color: HusTheme.Primary.colorTextBase
                            font.pixelSize: HusTheme.Primary.fontPrimarySize
                            font.weight: Font.Medium
                        }

                        HusColorPicker {
                            id: primaryColorPicker

                            Layout.preferredWidth: 150
                            Layout.preferredHeight: 34
                            defaultValue: AppSettings.primaryColor
                            showText: true
                            alphaEnabled: false
                            format: "hex"
                            title: qsTr("选择主题色")
                            Accessible.name: qsTr("主题色")
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 16

                        HusText {
                            Layout.fillWidth: true
                            text: qsTr("界面动画")
                            color: HusTheme.Primary.colorTextBase
                            font.pixelSize: HusTheme.Primary.fontPrimarySize
                            font.weight: Font.Medium
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
                Layout.preferredHeight: notificationLayout.implicitHeight + 40
                radius: 12
                color: HusTheme.Primary.colorBgContainer
                border.width: 1
                border.color: HusTheme.Primary.colorBorderSecondary

                ColumnLayout {
                    id: notificationLayout

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

                        HusText {
                            Layout.fillWidth: true
                            text: qsTr("系统通知")
                            color: HusTheme.Primary.colorTextBase
                            font.pixelSize: HusTheme.Primary.fontPrimarySize
                            font.weight: Font.Medium
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
                Layout.preferredHeight: diagnosticsLayout.implicitHeight + 40
                radius: 12
                color: HusTheme.Primary.colorBgContainer
                border.width: 1
                border.color: HusTheme.Primary.colorBorderSecondary

                ColumnLayout {
                    id: diagnosticsLayout

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

                        HusText {
                            Layout.fillWidth: true
                            text: qsTr("日志级别")
                            color: HusTheme.Primary.colorTextBase
                            font.pixelSize: HusTheme.Primary.fontPrimarySize
                            font.weight: Font.Medium
                        }

                        HusSelect {
                            id: logLevelSelect

                            Layout.preferredWidth: 150
                            Layout.preferredHeight: 34
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

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 16

                        HusText {
                            Layout.preferredWidth: 112
                            text: qsTr("日志文件路径")
                            color: HusTheme.Primary.colorTextBase
                            font.pixelSize: HusTheme.Primary.fontPrimarySize
                            font.weight: Font.Medium
                        }

                        HusInput {
                            id: logFilePathInput

                            Layout.fillWidth: true
                            Layout.preferredHeight: 34
                            selectByMouse: true
                            verticalAlignment: TextInput.AlignVCenter
                            contentDescription: qsTr("日志文件路径")
                        }

                        HusIconButton {
                            Layout.preferredWidth: 96
                            Layout.preferredHeight: 34
                            text: qsTr("浏览")
                            iconSource: HusIcon.FolderOpenOutlined
                            iconSize: 16
                            contentDescription: qsTr("选择日志文件路径")
                            onClicked: logFileDialog.open()
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: aboutLayout.implicitHeight + 40
                radius: 12
                color: HusTheme.Primary.colorFillQuaternary
                border.width: 1
                border.color: HusTheme.Primary.colorBorderSecondary

                RowLayout {
                    id: aboutLayout

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
                enabled: logFilePathInput.text.trim().length > 0
                         && themeModeSelect.currentIndex >= 0
                         && logLevelSelect.currentIndex >= 0
                contentDescription: qsTr("保存应用设置")
                onClicked: root.saveSettings()
            }
        }
    }
}
