pragma ComponentBehavior: Bound

import QtQuick
import HuskarUI.Basic

HusButton {
    id: root

    required property color swatchColor
    required property bool selected

    readonly property color checkColor: {
        const value = Qt.color(swatchColor);
        const luminance = value.r * 0.299 + value.g * 0.587 + value.b * 0.114;
        return luminance > 0.58 ? "#101522" : "#FFFFFF";
    }

    implicitWidth: 36
    implicitHeight: 36
    padding: 0
    type: HusButton.Type_Default
    colorBg: swatchColor
    colorBorder: selected ? HusTheme.Primary.colorTextBase : HusTheme.Primary.colorTextQuaternary
    borderWidth: selected ? 2 : 1
    radiusBg.all: 9
    contentDescription: qsTr("选择主题色 %1").arg(String(swatchColor).toUpperCase())

    Accessible.role: Accessible.RadioButton
    Accessible.name: contentDescription
    Accessible.checked: selected

    HusIconText {
        anchors.centerIn: parent
        visible: root.selected
        iconSource: HusIcon.CheckOutlined
        iconSize: 17
        colorIcon: root.checkColor
        Accessible.ignored: true
    }
}
