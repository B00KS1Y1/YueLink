import QtQuick
import HuskarUI.Basic

HusCopyableText {
    id: root

    required property string messageText

    text: root.messageText
    color: AppTheme.textPrimary
    wrapMode: Text.Wrap
    font.pixelSize: HusTheme.Primary.fontPrimarySize
}
