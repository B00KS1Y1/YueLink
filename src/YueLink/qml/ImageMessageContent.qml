import QtQuick
import HuskarUI.Basic

Column {
    id: root

    required property url fileUrl
    required property string fileName
    required property string caption
    required property bool previewAvailable
    required property real progress
    required property string deliveryStatus

    spacing: 8

    Image {
        width: parent.width
        height: status === Image.Ready ? 220 : 0
        source: root.previewAvailable ? root.fileUrl : ""
        sourceSize.width: 680
        sourceSize.height: 440
        asynchronous: true
        mipmap: true
        fillMode: Image.PreserveAspectFit
        visible: status === Image.Ready
        Accessible.name: qsTr("图片预览：%1").arg(root.fileName)
    }

    HusText {
        width: parent.width
        text: root.caption.length > 0
              ? root.caption
              : qsTr("图片：%1").arg(root.fileName)
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
