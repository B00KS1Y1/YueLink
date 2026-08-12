import QtQuick
import HuskarUI.Basic

Column {
    id: root

    required property string fileName
    required property string fileSizeText
    required property real progress
    required property string deliveryStatus

    spacing: 8

    HusText {
        width: parent.width
        text: qsTr("文件：%1\n%2").arg(root.fileName).arg(root.fileSizeText)
        color: HusTheme.Primary.colorTextBase
        wrapMode: Text.Wrap
        font.pixelSize: HusTheme.Primary.fontPrimarySize
    }

    Rectangle {
        width: parent.width
        height: 4
        radius: 2
        color: HusTheme.Primary.colorFillSecondary

        Rectangle {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: parent.width * Math.max(0, Math.min(1, root.progress))
            radius: parent.radius
            color: root.deliveryStatus === "failed"
                   ? "#D84A4A"
                   : root.deliveryStatus === "cancelled"
                     ? HusTheme.Primary.colorTextTertiary
                     : HusTheme.Primary.colorPrimary
        }
    }
}
