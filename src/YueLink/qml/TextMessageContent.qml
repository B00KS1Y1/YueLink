import QtQuick
import HuskarUI.Basic

HusCopyableText {
    id: root

    required property string messageText

    text: root.messageText
    color: HusTheme.Primary.colorTextBase
    wrapMode: Text.Wrap
    font.pixelSize: HusTheme.Primary.fontPrimarySize
}
