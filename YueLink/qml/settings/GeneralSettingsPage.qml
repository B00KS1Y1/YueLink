pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Dialogs
import QtQuick.Layouts
import HuskarUI.Basic
import "SettingsPathUtils.js" as PathUtils

Flickable {
    id: root

    contentWidth: width
    contentHeight: pageColumn.implicitHeight + 48
    boundsBehavior: Flickable.StopAtBounds
    clip: true
    ScrollBar.vertical: HusScrollBar { }

    function restoreDownloadDirectoryBinding(): void {
        downloadDirectoryInput.text = Qt.binding(() => AppSettings.application.downloadDirectory);
    }

    function commitPendingEdits(): bool {
        const saved = AppSettings.application.updateDownloadDirectory(downloadDirectoryInput.text);
        restoreDownloadDirectoryBinding();
        return saved;
    }

    function openDownloadDirectoryDialog(): void {
        const currentDirectoryUrl = PathUtils.localFileUrl(AppSettings.application.downloadDirectory);
        if (currentDirectoryUrl.toString().length > 0)
            downloadDirectoryDialog.currentFolder = currentDirectoryUrl;
        downloadDirectoryDialog.open();
    }

    FolderDialog {
        id: downloadDirectoryDialog

        title: qsTr("选择下载目录")
        onAccepted: {
            AppSettings.application.updateDownloadDirectory(PathUtils.localFilePath(selectedFolder));
            root.restoreDownloadDirectoryBinding();
        }
    }

    ColumnLayout {
        id: pageColumn

        x: Math.max(24, (root.width - width) * 0.5)
        y: 8
        width: Math.min(900, root.width - 48)
        spacing: 20

        SettingsGroup {
            Layout.fillWidth: true
            title: qsTr("消息通知")

            SettingsRow {
                title: qsTr("系统通知")
                description: qsTr("收到新消息时显示桌面通知")
                controlWidth: 64
                last: true

                HusSwitch {
                    property bool initialized: false

                    anchors.centerIn: parent
                    width: 48
                    height: 28
                    checked: AppSettings.application.notificationsEnabled
                    contentDescription: qsTr("启用系统通知")
                    Component.onCompleted: initialized = true
                    onToggled: {
                        if (!initialized || checked === AppSettings.application.notificationsEnabled)
                            return;
                        AppSettings.application.updateNotificationsEnabled(checked);
                        checked = Qt.binding(() => AppSettings.application.notificationsEnabled);
                    }
                }
            }
        }

        SettingsGroup {
            Layout.fillWidth: true
            title: qsTr("文件接收")

            SettingsRow {
                title: qsTr("下载目录")
                description: qsTr("接收到的文件将保存到此位置")
                controlWidth: 430
                last: true

                RowLayout {
                    anchors.fill: parent
                    spacing: 8

                    HusInput {
                        id: downloadDirectoryInput

                        Layout.fillWidth: true
                        Layout.preferredHeight: 38
                        text: AppSettings.application.downloadDirectory
                        selectByMouse: true
                        verticalAlignment: TextInput.AlignVCenter
                        contentDescription: qsTr("下载目录")
                        onEditingFinished: root.commitPendingEdits()
                    }

                    HusIconButton {
                        Layout.preferredWidth: 42
                        Layout.preferredHeight: 38
                        padding: 0
                        iconSource: HusIcon.FolderOpenOutlined
                        iconSize: 17
                        contentDescription: qsTr("选择下载目录")
                        onClicked: root.openDownloadDirectoryDialog()
                    }
                }
            }
        }
    }
}
