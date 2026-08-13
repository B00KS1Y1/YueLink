import QtQuick
import HuskarUI.Basic
import "EmojiCatalog.mjs" as EmojiCatalog

Item {
    id: root

    required property string emojiPackageId
    required property string emojiId
    required property string fallbackText
    readonly property url emojiSource: EmojiCatalog.sourceFor(
                                           root.emojiPackageId,
                                           root.emojiId)

    implicitWidth: 64
    implicitHeight: 64
    Accessible.role: Accessible.StaticText
    Accessible.name: qsTr("表情：%1").arg(root.fallbackText)

    Image {
        id: emojiImage

        anchors.fill: parent
        visible: status === Image.Ready
        source: root.emojiSource
        sourceSize.width: 64
        sourceSize.height: 64
        fillMode: Image.PreserveAspectFit
        asynchronous: true
        Accessible.ignored: true
    }

    HusText {
        anchors.centerIn: parent
        visible: emojiImage.status !== Image.Ready
        text: root.fallbackText
        color: AppTheme.textPrimary
        font.pixelSize: 32
        horizontalAlignment: Text.AlignHCenter
        Accessible.ignored: true
    }
}
