import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import HuskarUI.Basic

HusWindow {
    id: root

    width: 440
    height: 520
    minimumWidth: 400
    minimumHeight: 460
    title: qsTr("关于 YueLink")
    visible: false
    color: AppTheme.canvas
    captionBar.color: AppTheme.canvas
    captionBar.showMinimizeButton: false
    captionBar.showMaximizeButton: false
    captionBar.winIconDelegate: Item {
        implicitWidth: 22
        implicitHeight: 22

        Image {
            anchors.centerIn: parent
            width: 16
            height: 16
            source: "qrc:/yuelink/assets/yuelink-app-icon.png"
            sourceSize.width: 16
            sourceSize.height: 16
            fillMode: Image.PreserveAspectFit
            Accessible.ignored: true
        }
    }
    captionBar.closeCallback: () => root.close()

    Shortcut {
        sequence: StandardKey.Cancel
        onActivated: root.close()
    }

    Rectangle {
        anchors.fill: parent
        color: AppTheme.canvas

        ColumnLayout {
            anchors.centerIn: parent
            width: Math.min(parent.width - 48, 380)
            spacing: 14

            Rectangle {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 88
                Layout.preferredHeight: 88
                radius: 22
                color: AppTheme.surfaceElevated
                border.width: 1
                border.color: AppTheme.border

                Image {
                    anchors.centerIn: parent
                    width: 60
                    height: 60
                    source: "qrc:/yuelink/assets/yuelink-app-icon.png"
                    sourceSize.width: 60
                    sourceSize.height: 60
                    fillMode: Image.PreserveAspectFit
                    Accessible.ignored: true
                }
            }

            HusText {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                text: qsTr("YueLink")
                color: AppTheme.textPrimary
                font.pixelSize: HusTheme.Primary.fontPrimarySizeHeading3
                font.weight: Font.Bold
            }

            HusText {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                text: qsTr("局域网即时聊天与文件传输工具")
                color: AppTheme.textSecondary
                font.pixelSize: HusTheme.Primary.fontPrimarySize
            }

            HusText {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                text: qsTr("版本：0.1")
                color: AppTheme.textPrimary
                font.pixelSize: HusTheme.Primary.fontPrimarySize
            }

            HusText {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                text: qsTr("作者：系梨xili")
                color: AppTheme.textPrimary
                font.pixelSize: HusTheme.Primary.fontPrimarySize
            }

            HusText {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                text: qsTr("欢迎在局域网内与好友安全、快速地沟通与传输文件。")
                color: AppTheme.textSecondary
                font.pixelSize: HusTheme.Primary.fontPrimarySize
            }
        }
    }
}
