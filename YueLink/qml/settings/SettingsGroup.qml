pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic

ColumnLayout {
    id: root

    required property string title
    default property alias content: groupContent.data

    spacing: 10

    RowLayout {
        Layout.fillWidth: true
        spacing: 10

        Rectangle {
            Layout.preferredWidth: 4
            Layout.preferredHeight: 22
            radius: 2
            color: HusTheme.Primary.colorPrimary
            Accessible.ignored: true
        }

        HusText {
            Layout.fillWidth: true
            text: root.title
            color: HusTheme.Primary.colorTextBase
            font.pixelSize: HusTheme.Primary.fontPrimarySizeHeading4
            font.weight: Font.DemiBold
        }
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: groupContent.implicitHeight
        radius: HusTheme.Primary.radiusPrimary
        color: HusTheme.Primary.colorBgBase
        border.width: 1
        border.color: HusTheme.Primary.colorSplit

        ColumnLayout {
            id: groupContent

            anchors.fill: parent
            spacing: 0
        }
    }
}
