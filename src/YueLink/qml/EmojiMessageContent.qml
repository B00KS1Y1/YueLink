import QtQuick
import HuskarUI.Basic

HusText {
    id: root

    required property string fallbackText

    text: root.fallbackText
    color: HusTheme.Primary.colorTextBase
    font.pixelSize: 42
    horizontalAlignment: Text.AlignHCenter
    Accessible.name: qsTr("表情：%1").arg(root.fallbackText)
}
