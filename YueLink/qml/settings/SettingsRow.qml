pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic

Item {
    id: root

    required property string title
    property string description: ""
    property string badgeText: ""
    property int controlWidth: 280
    property bool last: false
    default property alias control: controlHost.data
    readonly property int minimumRowHeight: description.length > 0 ? 76 : 62

    Layout.fillWidth: true
    Layout.minimumHeight: root.minimumRowHeight
    Layout.preferredHeight: Math.max(root.minimumRowHeight, labelColumn.implicitHeight + 24)

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        spacing: 24

        ColumnLayout {
            id: labelColumn

            Layout.fillWidth: true
            spacing: 3

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                HusText {
                    Layout.fillWidth: true
                    text: root.title
                    color: HusTheme.Primary.colorTextBase
                    font.pixelSize: HusTheme.Primary.fontPrimarySize
                    font.weight: Font.Medium
                    elide: Text.ElideRight
                }

                HusText {
                    Layout.preferredWidth: visible ? implicitWidth : 0
                    visible: root.badgeText.length > 0
                    text: root.badgeText
                    color: HusTheme.Primary.colorPrimary
                    font.pixelSize: Math.max(12, HusTheme.Primary.fontPrimarySize - 2)
                    font.weight: Font.Medium
                }
            }

            HusText {
                Layout.fillWidth: true
                visible: root.description.length > 0
                text: root.description
                color: HusTheme.Primary.colorTextQuaternary
                font.pixelSize: Math.max(12, HusTheme.Primary.fontPrimarySize - 2)
                elide: Text.ElideRight
            }
        }

        Item {
            id: controlHost

            Layout.preferredWidth: Math.min(root.controlWidth, Math.max(64, root.width * 0.52))
            Layout.minimumWidth: Math.min(root.controlWidth, 64)
            Layout.preferredHeight: 40
        }
    }

    Rectangle {
        anchors.left: root.left
        anchors.right: root.right
        anchors.bottom: root.bottom
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        height: 1
        visible: !root.last
        color: HusTheme.Primary.colorSplit
        Accessible.ignored: true
    }
}
