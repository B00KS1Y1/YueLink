import QtQuick
import QtQuick.Effects
import HuskarUI.Basic

Item {
    id: root

    property int size: 30
    property url imageSource
    property string textSource: ""
    property color colorBg: HusTheme.Primary.colorTextQuaternary
    property color colorText: "white"
    property int textSize: HusAvatar.Size_Fixed

    readonly property real effectiveDevicePixelRatio: Math.max(
                                                          1,
                                                          Screen.devicePixelRatio)

    implicitWidth: size
    implicitHeight: size
    width: size
    height: size

    HusAvatar {
        anchors.fill: parent
        size: root.size
        textSource: root.textSource
        colorBg: root.colorBg
        colorText: root.colorText
        textSize: root.textSize
        visible: avatarImage.status !== Image.Ready
    }

    Rectangle {
        anchors.fill: parent
        radius: Math.min(width, height) * 0.5
        color: root.colorBg
        visible: avatarImage.status === Image.Ready
        Accessible.ignored: true
    }

    Image {
        id: avatarImage

        anchors.fill: parent
        source: root.imageSource
        sourceSize.width: Math.max(
                              1,
                              Math.ceil(width
                                        * root.effectiveDevicePixelRatio))
        sourceSize.height: Math.max(
                               1,
                               Math.ceil(height
                                         * root.effectiveDevicePixelRatio))
        fillMode: Image.PreserveAspectCrop
        asynchronous: true
        cache: true
        smooth: true
        autoTransform: true
        layer.enabled: true
        visible: false
    }

    Rectangle {
        id: avatarMask

        anchors.fill: parent
        radius: Math.min(width, height) * 0.5
        layer.enabled: true
        visible: false
    }

    MultiEffect {
        anchors.fill: avatarImage
        source: avatarImage
        maskEnabled: true
        maskSource: avatarMask
        visible: avatarImage.status === Image.Ready
    }
}
