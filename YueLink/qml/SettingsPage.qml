pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic
import "settings" as SettingsUi

Item {
    id: root

    property int currentCategoryIndex: 0
    property int pendingCategoryIndex: -1
    property int categorySwitchDirection: 1

    readonly property bool animationEnabled: HusTheme.animationEnabled
    readonly property bool compactNavigation: width < 760
    readonly property int navigationWidth: compactNavigation ? 76 : 204
    readonly property string currentCategoryKey: categoryModel[currentCategoryIndex].key
    readonly property string currentCategoryTitle: categoryModel[currentCategoryIndex].label
    readonly property var currentSettingsModel: currentCategoryKey === "general"
                                                    ? AppSettings.application
                                                    : currentCategoryKey === "appearance"
                                                      ? AppSettings.theme : AppSettings.log
    readonly property string currentCategoryDescription: {
        switch (currentCategoryKey) {
        case "general":
            return qsTr("管理消息通知和接收文件的保存位置");
        case "appearance":
            return qsTr("调整主题、颜色和界面显示方式");
        case "advanced":
            return qsTr("配置诊断日志和开发辅助选项");
        default:
            return "";
        }
    }
    readonly property var categoryModel: [
        {
            "key": "general",
            "label": qsTr("通用"),
            "iconSource": HusIcon.SettingOutlined
        },
        {
            "key": "appearance",
            "label": qsTr("外观"),
            "iconSource": HusIcon.SkinOutlined
        },
        {
            "key": "advanced",
            "label": qsTr("高级"),
            "iconSource": HusIcon.BugOutlined
        },
        {
            "key": "adbout",
            "label": qsTr("关于"),
            "iconSource": HusIcon.InfoCircleOutlined
        }
    ]

    signal closeRequested()

    function commitCurrentPage(): bool {
        if (currentCategoryKey === "general")
            return generalSettingsPage.commitPendingEdits();
        if (currentCategoryKey === "advanced")
            return advancedSettingsPage.commitPendingEdits();
        return true;
    }

    function selectCategory(index: int): void {
        if (currentCategoryIndex === index || categorySwitchAnimation.running)
            return;
        if (!commitCurrentPage())
            return;
        if (!animationEnabled) {
            currentCategoryIndex = index;
            return;
        }

        pendingCategoryIndex = index;
        categorySwitchDirection = index > currentCategoryIndex ? 1 : -1;
        categorySwitchAnimation.start();
    }

    function completeCategorySwitch(): void {
        categorySwitchAnimation.stop();
        if (pendingCategoryIndex >= 0)
            currentCategoryIndex = pendingCategoryIndex;
        pendingCategoryIndex = -1;
        pageHeader.opacity = 1;
        settingsPageStack.opacity = 1;
        settingsPageStack.scale = 1;
        settingsPageStack.x = 0;
    }

    function requestClose(): void {
        if (!commitCurrentPage())
            return;
        closeRequested();
    }

    component SettingsNavigationItem: HusIconButton {
        id: navigationItem

        required property string label
        required property bool selected
        property bool compact: false

        implicitHeight: 46
        text: compact ? "" : label
        iconPosition: HusIconButton.Position_Start
        iconSpacing: 12
        iconSize: 18
        leftPadding: compact ? 0 : 14
        rightPadding: compact ? 0 : 14
        type: HusButton.Type_Default
        colorBg: selected ? HusTheme.Primary.colorPrimary
                          : hovered ? HusTheme.Primary.colorFillSecondary : "transparent"
        colorText: selected ? "white" : HusTheme.Primary.colorTextTertiary
        colorIcon: colorText
        colorBorder: visualFocus ? HusTheme.Primary.colorPrimary : "transparent"
        borderWidth: visualFocus ? 2 : 0
        radiusBg.all: 10
        contentDescription: label

        Accessible.role: Accessible.RadioButton
        Accessible.name: label
        Accessible.checked: selected

        Rectangle {
            anchors.left: parent.left
            anchors.leftMargin: 5
            anchors.verticalCenter: parent.verticalCenter
            width: 3
            height: 18
            radius: 2
            color: "white"
            opacity: navigationItem.selected ? 1 : 0
            scale: navigationItem.selected ? 1 : 0.55
            z: 1
            Accessible.ignored: true

            Behavior on opacity {
                enabled: navigationItem.animationEnabled
                OpacityAnimator {
                    duration: HusTheme.Primary.durationFast
                    easing.type: Easing.OutCubic
                }
            }

            Behavior on scale {
                enabled: navigationItem.animationEnabled
                ScaleAnimator {
                    duration: HusTheme.Primary.durationMid
                    easing.type: Easing.OutBack
                }
            }
        }
    }

    onAnimationEnabledChanged: {
        if (!animationEnabled && categorySwitchAnimation.running)
            completeCategorySwitch();
    }

    Shortcut {
        sequences: [StandardKey.Cancel]
        onActivated: root.requestClose()
    }

    SequentialAnimation {
        id: categorySwitchAnimation

        ParallelAnimation {
            OpacityAnimator {
                target: pageHeader
                from: 1
                to: 0
                duration: HusTheme.Primary.durationFast
                easing.type: Easing.InCubic
            }

            OpacityAnimator {
                target: settingsPageStack
                from: 1
                to: 0
                duration: HusTheme.Primary.durationFast
                easing.type: Easing.InCubic
            }

            XAnimator {
                target: settingsPageStack
                from: 0
                to: -root.categorySwitchDirection * 14
                duration: HusTheme.Primary.durationFast
                easing.type: Easing.InCubic
            }

            ScaleAnimator {
                target: settingsPageStack
                from: 1
                to: 0.985
                duration: HusTheme.Primary.durationFast
                easing.type: Easing.InCubic
            }
        }

        ScriptAction {
            script: {
                root.currentCategoryIndex = root.pendingCategoryIndex;
                settingsPageStack.x = root.categorySwitchDirection * 20;
            }
        }

        ParallelAnimation {
            OpacityAnimator {
                target: pageHeader
                from: 0
                to: 1
                duration: HusTheme.Primary.durationMid
                easing.type: Easing.OutCubic
            }

            OpacityAnimator {
                target: settingsPageStack
                from: 0
                to: 1
                duration: HusTheme.Primary.durationMid
                easing.type: Easing.OutCubic
            }

            XAnimator {
                target: settingsPageStack
                from: root.categorySwitchDirection * 20
                to: 0
                duration: HusTheme.Primary.durationMid
                easing.type: Easing.OutCubic
            }

            ScaleAnimator {
                target: settingsPageStack
                from: 0.985
                to: 1
                duration: HusTheme.Primary.durationMid
                easing.type: Easing.OutCubic
            }
        }

        ScriptAction {
            script: root.pendingCategoryIndex = -1
        }
    }

    Rectangle {
        anchors.fill: parent
        color: HusTheme.Primary.colorBgBase
        Accessible.ignored: true
    }

    Rectangle {
        id: navigationPanel

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: root.navigationWidth
        color: HusTheme.Primary.colorFillTertiary
        border.width: 1
        border.color: HusTheme.Primary.colorSplit

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 12

            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 48
                spacing: 8

                HusIconButton {
                    Layout.preferredWidth: 42
                    Layout.preferredHeight: 42
                    type: HusButton.Type_Text
                    iconSource: HusIcon.ArrowLeftOutlined
                    iconSize: 18
                    padding: 0
                    radiusBg.all: 9
                    contentDescription: qsTr("返回应用")
                    onClicked: root.requestClose()
                }

                HusText {
                    Layout.fillWidth: true
                    visible: !root.compactNavigation
                    text: qsTr("应用设置")
                    color: HusTheme.Primary.colorTextBase
                    font.pixelSize: HusTheme.Primary.fontPrimarySize
                    font.weight: Font.Medium
                    elide: Text.ElideRight
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: HusTheme.Primary.colorSplit
                Accessible.ignored: true
            }

            Column {
                id: navigationItemsColumn

                Layout.fillWidth: true
                spacing: 4

                Repeater {
                    model: root.categoryModel

                    delegate: SettingsNavigationItem {
                        required property var modelData
                        required property int index

                        width: navigationItemsColumn.width
                        label: modelData.label
                        iconSource: modelData.iconSource
                        compact: root.compactNavigation
                        selected: root.currentCategoryIndex === index
                        onClicked: root.selectCategory(index)
                    }
                }
            }

            Item {
                Layout.fillHeight: true
            }

            HusText {
                Layout.fillWidth: true
                visible: !root.compactNavigation
                text: qsTr("YueLink 0.1")
                color: HusTheme.Primary.colorTextQuaternary
                font.pixelSize: Math.max(12, HusTheme.Primary.fontPrimarySize - 2)
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    Rectangle {
        id: contentPanel

        anchors.left: navigationPanel.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        color: HusTheme.Primary.colorBgBase

        Item {
            id: pageHeader

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: 94

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 32
                anchors.rightMargin: 32
                spacing: 20

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 5

                    HusText {
                        Layout.fillWidth: true
                        text: root.currentCategoryTitle
                        color: HusTheme.Primary.colorTextBase
                        font.pixelSize: HusTheme.Primary.fontPrimarySizeHeading2
                        font.weight: Font.DemiBold
                        elide: Text.ElideRight
                    }

                    HusText {
                        Layout.fillWidth: true
                        text: root.currentCategoryDescription
                        color: HusTheme.Primary.colorTextTertiary
                        font.pixelSize: HusTheme.Primary.fontPrimarySize
                        elide: Text.ElideRight
                    }
                }

                SettingsUi.SettingsSaveStatus {
                    Layout.preferredWidth: visible ? implicitWidth : 0
                    Layout.maximumWidth: Math.min(380, pageHeader.width * 0.42)
                    settingsModel: root.currentSettingsModel
                }
            }
        }

        Item {
            id: pageViewport

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: pageHeader.bottom
            anchors.bottom: parent.bottom
            clip: categorySwitchAnimation.running

            StackLayout {
                id: settingsPageStack

                x: 0
                y: 0
                width: parent.width
                height: parent.height
                enabled: !categorySwitchAnimation.running
                currentIndex: root.currentCategoryIndex
                transformOrigin: Item.Center

                SettingsUi.GeneralSettingsPage {
                    id: generalSettingsPage

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                SettingsUi.AppearanceSettingsPage {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                SettingsUi.AdvancedSettingsPage {
                    id: advancedSettingsPage

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }
        }
    }
}
