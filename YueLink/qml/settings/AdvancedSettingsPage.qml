pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Dialogs
import QtQuick.Layouts
import HuskarUI.Basic
import "SettingsPathUtils.js" as PathUtils

Flickable {
    id: root

    readonly property int logLevelIndex: AppSettings.log.level === "trace" ? 0
                                         : AppSettings.log.level === "debug" ? 1
                                         : AppSettings.log.level === "info" ? 2
                                         : AppSettings.log.level === "warn" ? 3
                                         : AppSettings.log.level === "error" ? 4
                                         : AppSettings.log.level === "critical" ? 5 : 6

    contentWidth: width
    contentHeight: pageColumn.implicitHeight + 48
    boundsBehavior: Flickable.StopAtBounds
    clip: true
    ScrollBar.vertical: HusScrollBar { }

    function restoreLogFilePathBinding(): void {
        logFilePathInput.text = Qt.binding(() => AppSettings.log.filePath);
    }

    function commitPendingEdits(): bool {
        const saved = AppSettings.log.updateFilePath(logFilePathInput.text);
        restoreLogFilePathBinding();
        return saved;
    }

    function openLogFileDialog(): void {
        const currentDirectoryUrl = PathUtils.localFileUrl(
                                      PathUtils.containingDirectoryPath(AppSettings.log.filePath));
        if (currentDirectoryUrl.toString().length > 0)
            logFileDialog.currentFolder = currentDirectoryUrl;
        logFileDialog.open();
    }

    FileDialog {
        id: logFileDialog

        title: qsTr("选择日志文件")
        fileMode: FileDialog.SaveFile
        defaultSuffix: "log"
        nameFilters: [qsTr("日志文件 (*.log)"), qsTr("所有文件 (*)")]
        onAccepted: {
            AppSettings.log.updateFilePath(PathUtils.localFilePath(selectedFile));
            root.restoreLogFilePathBinding();
        }
    }

    ColumnLayout {
        id: pageColumn

        x: Math.max(24, (root.width - width) * 0.5)
        y: 8
        width: Math.min(900, root.width - 48)
        spacing: 20

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: diagnosticHint.implicitHeight + 24
            radius: HusTheme.Primary.radiusPrimary
            color: HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, HusTheme.isDark ? 0.18 : 0.1)
            border.width: 1
            border.color: HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, HusTheme.isDark ? 0.28 : 0.16)

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                spacing: 10

                HusIconText {
                    Layout.preferredWidth: 20
                    iconSource: HusIcon.InfoCircleOutlined
                    iconSize: 18
                    colorIcon: HusTheme.Primary.colorPrimary
                    Accessible.ignored: true
                }

                HusText {
                    id: diagnosticHint

                    Layout.fillWidth: true
                    text: qsTr("这些选项主要用于问题诊断。")
                    color: HusTheme.Primary.colorTextTertiary
                    font.pixelSize: HusTheme.Primary.fontPrimarySize
                    wrapMode: Text.WordWrap
                }
            }
        }

        SettingsGroup {
            Layout.fillWidth: true
            title: qsTr("诊断日志")

            SettingsRow {
                title: qsTr("日志级别")
                description: qsTr("级别越低，记录的诊断信息越详细")
                controlWidth: 180

                HusSelect {
                    id: logLevelControl

                    anchors.fill: parent
                    clearEnabled: false
                    currentIndex: root.logLevelIndex
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
                    onActivated: {
                        if (currentValue === undefined || currentValue === null)
                            return;
                        AppSettings.log.updateLevel(currentValue);
                        currentIndex = Qt.binding(() => root.logLevelIndex);
                    }
                }
            }

            SettingsRow {
                title: qsTr("日志文件路径")
                description: qsTr("诊断日志保存到此文件")
                controlWidth: 430

                RowLayout {
                    anchors.fill: parent
                    spacing: 8

                    HusInput {
                        id: logFilePathInput

                        Layout.fillWidth: true
                        Layout.preferredHeight: 38
                        text: AppSettings.log.filePath
                        selectByMouse: true
                        verticalAlignment: TextInput.AlignVCenter
                        contentDescription: qsTr("日志文件路径")
                        onEditingFinished: root.commitPendingEdits()
                    }

                    HusIconButton {
                        Layout.preferredWidth: 42
                        Layout.preferredHeight: 38
                        padding: 0
                        iconSource: HusIcon.FolderOpenOutlined
                        iconSize: 17
                        contentDescription: qsTr("选择日志文件路径")
                        onClicked: root.openLogFileDialog()
                    }
                }
            }

            SettingsRow {
                title: qsTr("记录源码位置")
                description: qsTr("在日志中附带源码文件路径和行号")
                controlWidth: 64

                HusSwitch {
                    property bool initialized: false

                    anchors.centerIn: parent
                    width: 48
                    height: 28
                    checked: AppSettings.log.sourceLocationEnabled
                    contentDescription: qsTr("在日志中记录源码位置")
                    Component.onCompleted: initialized = true
                    onToggled: {
                        if (!initialized || checked === AppSettings.log.sourceLocationEnabled)
                            return;
                        AppSettings.log.updateSourceLocationEnabled(checked);
                        checked = Qt.binding(() => AppSettings.log.sourceLocationEnabled);
                    }
                }
            }

            SettingsRow {
                title: qsTr("独立线程写入")
                description: qsTr("通过专用线程异步写入日志文件")
                controlWidth: 64
                last: true

                HusSwitch {
                    property bool initialized: false

                    anchors.centerIn: parent
                    width: 48
                    height: 28
                    checked: AppSettings.log.separateThreadEnabled
                    contentDescription: qsTr("通过独立线程异步写入日志")
                    Component.onCompleted: initialized = true
                    onToggled: {
                        if (!initialized || checked === AppSettings.log.separateThreadEnabled)
                            return;
                        AppSettings.log.updateSeparateThreadEnabled(checked);
                        checked = Qt.binding(() => AppSettings.log.separateThreadEnabled);
                    }
                }
            }
        }
    }
}
