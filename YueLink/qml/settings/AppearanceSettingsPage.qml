pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import HuskarUI.Basic

Flickable {
    id: root

    readonly property var colorPresets: [
        "#4F7CFF",
        "#16A6B6",
        "#22A06B",
        "#E58A1F",
        "#8B5CF6"
    ]
    readonly property int themeModeIndex: AppSettings.theme.mode === "system" ? 0
                                          : AppSettings.theme.mode === "light" ? 1 : 2

    contentWidth: width
    contentHeight: pageColumn.implicitHeight + 48
    boundsBehavior: Flickable.StopAtBounds
    clip: true
    ScrollBar.vertical: HusScrollBar { }

    ColumnLayout {
        id: pageColumn

        x: Math.max(24, (root.width - width) * 0.5)
        y: 8
        width: Math.min(900, root.width - 48)
        spacing: 20

        SettingsGroup {
            Layout.fillWidth: true
            title: qsTr("主题")

            SettingsRow {
                title: qsTr("主题模式")
                description: qsTr("选择浅色、深色或跟随系统")
                controlWidth: 360

                HusSegmented {
                    id: themeModeControl

                    property bool initialized: false

                    anchors.fill: parent
                    block: true
                    currentIndex: root.themeModeIndex
                    options: [
                        { "label": qsTr("跟随系统"), "value": "system" },
                        { "label": qsTr("浅色"), "value": "light" },
                        { "label": qsTr("深色"), "value": "dark" }
                    ]
                    Component.onCompleted: initialized = true
                    onCurrentValueChanged: {
                        if (!initialized || currentValue === undefined || currentValue === null
                                || currentValue === AppSettings.theme.mode)
                            return;
                        AppSettings.theme.updateMode(currentValue);
                        currentIndex = Qt.binding(() => root.themeModeIndex);
                    }
                }
            }

            SettingsRow {
                title: qsTr("主题色")
                description: qsTr("选择预设颜色或创建自定义颜色")
                controlWidth: 400

                RowLayout {
                    anchors.fill: parent
                    spacing: 8

                    Repeater {
                        model: root.colorPresets

                        delegate: ThemeColorSwatch {
                            required property var modelData

                            Layout.preferredWidth: 36
                            Layout.preferredHeight: 36
                            swatchColor: modelData
                            selected: String(modelData).toUpperCase() === AppSettings.theme.primaryColor.toUpperCase()
                            onClicked: AppSettings.theme.updatePrimaryColor(String(modelData))
                        }
                    }

                    HusColorPicker {
                        id: themeColorPicker

                        Layout.fillWidth: true
                        Layout.minimumWidth: 104
                        Layout.preferredHeight: 38
                        autoChange: false
                        changeValue: AppSettings.theme.primaryColor
                        defaultValue: AppSettings.theme.primaryColor
                        showText: true
                        alphaEnabled: false
                        format: "hex"
                        title: qsTr("自定义主题色")
                        Accessible.name: qsTr("自定义主题色")
                        onChange: color => {
                            AppSettings.theme.updatePrimaryColor(toHexString(color));
                            changeValue = Qt.binding(() => AppSettings.theme.primaryColor);
                            defaultValue = Qt.binding(() => AppSettings.theme.primaryColor);
                        }
                    }
                }
            }

            SettingsRow {
                title: qsTr("界面动画")
                description: qsTr("显示页面切换和控件反馈动画")
                controlWidth: 64
                last: true

                HusSwitch {
                    property bool initialized: false

                    anchors.centerIn: parent
                    width: 48
                    height: 28
                    checked: AppSettings.theme.animationsEnabled
                    contentDescription: qsTr("启用界面动画")
                    Component.onCompleted: initialized = true
                    onToggled: {
                        if (!initialized || checked === AppSettings.theme.animationsEnabled)
                            return;
                        AppSettings.theme.updateAnimationsEnabled(checked);
                        checked = Qt.binding(() => AppSettings.theme.animationsEnabled);
                    }
                }
            }
        }
    }
}
