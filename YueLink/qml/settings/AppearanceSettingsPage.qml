pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Dialogs
import QtQuick.Layouts
import HuskarUI.Basic
import "SettingsPathUtils.js" as PathUtils

Flickable {
    id: root

    property color controlSurfaceColor: HusThemeFunctions.alpha(
                                            HusTheme.Primary.colorBgBase, 0.42)

    readonly property var colorPresets: [
        "#4F7CFF",
        "#16A6B6",
        "#22A06B",
        "#E58A1F",
        "#8B5CF6"
    ]
    readonly property int themeModeIndex: AppSettings.theme.mode === "system" ? 0
                                          : AppSettings.theme.mode === "light" ? 1 : 2
    readonly property int backgroundImageIndex: {
        const currentSource = AppSettings.theme.backgroundImage.toString();
        const backgrounds = AppSettings.theme.backgroundImages;
        for (let index = 0; index < backgrounds.length; ++index) {
            if (backgrounds[index].value.toString() === currentSource)
                return index;
        }
        return -1;
    }
    readonly property bool backgroundImageRemovable: root.backgroundImageIndex >= 0
                                                      && AppSettings.theme.backgroundImages.length > 1
    readonly property string selectedBackgroundImageName: root.backgroundImageIndex >= 0
                                                           ? AppSettings.theme.backgroundImages[root.backgroundImageIndex].label : ""
    readonly property url selectedBackgroundImageSource: root.backgroundImageIndex >= 0
                                                         ? AppSettings.theme.backgroundImages[root.backgroundImageIndex].value : ""

    contentWidth: width
    contentHeight: pageColumn.implicitHeight + 48
    boundsBehavior: Flickable.StopAtBounds
    clip: true
    ScrollBar.vertical: HusScrollBar { }

    function updateBackgroundOpacity(percent: real): void {
        const normalizedPercent = Math.max(0, Math.min(100, Math.round(percent)));
        AppSettings.theme.updateBackgroundOpacity(normalizedPercent / 100.0);
        backgroundOpacityInput.value = Qt.binding(
                    () => Math.round(AppSettings.theme.backgroundOpacity * 100));
    }

    FileDialog {
        id: backgroundImageDialog

        title: qsTr("导入背景图片")
        fileMode: FileDialog.OpenFile
        nameFilters: [
            qsTr("图片文件 (*.png *.jpg *.jpeg *.bmp *.webp)"),
            qsTr("所有文件 (*)")
        ]
        onAccepted: {
            backgroundNameModal.imageSource = selectedFile;
            backgroundNameModal.imageName = PathUtils.fileBaseName(selectedFile);
            backgroundNameModal.importAttempted = false;
            backgroundNameModal.open();
        }
    }

    HusModal {
        id: backgroundNameModal

        property url imageSource
        property string imageName: ""
        property bool importAttempted: false

        width: Math.min(440, root.width - 48)
        title: qsTr("命名背景图片")
        confirmText: qsTr("添加")
        cancelText: qsTr("取消")
        maskClosable: true
        bodyDelegate: Column {
            height: implicitHeight
            spacing: 8

            HusText {
                width: parent.width
                text: qsTr("图片将复制到 YueLink 背景库，并显示在背景下拉框中。")
                color: HusTheme.Primary.colorTextTertiary
                font.pixelSize: HusTheme.Primary.fontPrimarySize
                wrapMode: Text.WordWrap
            }

            HusInput {
                id: backgroundNameInput

                width: parent.width
                height: 36
                maximumLength: 64
                clearEnabled: "active"
                text: backgroundNameModal.imageName
                placeholderText: qsTr("输入背景名称")
                contentDescription: qsTr("背景名称")
                onTextEdited: backgroundNameModal.imageName = text
                Keys.onReturnPressed: {
                    if (backgroundNameModal.imageName.trim().length > 0)
                        backgroundNameModal.confirm();
                }

                Connections {
                    target: backgroundNameModal
                    function onOpened(): void {
                        Qt.callLater(() => backgroundNameInput.forceActiveFocus());
                    }
                }
            }

            HusText {
                width: parent.width
                visible: backgroundNameModal.importAttempted
                         && AppSettings.theme.errorMessage.length > 0
                text: AppSettings.theme.errorMessage
                color: HusTheme.Primary.colorError
                font.pixelSize: Math.max(12, HusTheme.Primary.fontPrimarySize - 1)
                wrapMode: Text.WordWrap
            }
        }
        confirmButtonDelegate: HusButton {
            text: qsTr("添加")
            type: HusButton.Type_Primary
            enabled: backgroundNameModal.imageName.trim().length > 0
            onClicked: backgroundNameModal.confirm()
        }
        onConfirm: {
            importAttempted = true;
            if (AppSettings.theme.importBackgroundImage(imageName, imageSource))
                close();
        }
        onCancel: close()
        onClosed: {
            imageSource = "";
            imageName = "";
            importAttempted = false;
        }
    }

    HusModal {
        id: backgroundDeleteModal

        property url imageSource
        property string imageName: ""
        property bool deleteAttempted: false

        width: Math.min(440, root.width - 48)
        title: qsTr("删除背景图片")
        iconSource: HusIcon.DeleteOutlined
        colorIcon: HusTheme.Primary.colorError
        confirmText: qsTr("删除")
        cancelText: qsTr("取消")
        maskClosable: true
        bodyDelegate: Column {
            height: implicitHeight
            spacing: 8

            HusText {
                width: parent.width
                text: qsTr("确定删除“%1”吗？该项将从背景下拉框中隐藏，但配置记录和图片文件会保留。删除后将自动切换到其他背景。")
                      .arg(backgroundDeleteModal.imageName)
                color: HusTheme.Primary.colorTextTertiary
                font.pixelSize: HusTheme.Primary.fontPrimarySize
                wrapMode: Text.WordWrap
            }

            HusText {
                width: parent.width
                visible: backgroundDeleteModal.deleteAttempted
                         && AppSettings.theme.errorMessage.length > 0
                text: AppSettings.theme.errorMessage
                color: HusTheme.Primary.colorError
                font.pixelSize: Math.max(12, HusTheme.Primary.fontPrimarySize - 1)
                wrapMode: Text.WordWrap
            }
        }
        onConfirm: {
            deleteAttempted = true;
            if (AppSettings.theme.removeBackgroundImage(imageSource))
                close();
        }
        onCancel: close()
        onClosed: {
            imageSource = "";
            imageName = "";
            deleteAttempted = false;
        }
    }

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
                    colorBg: root.controlSurfaceColor
                    colorIndicatorBg: HusThemeFunctions.alpha(
                                          HusTheme.Primary.colorTextBase,
                                          HusTheme.isDark ? 0.14 : 0.1)
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
                        colorBg: root.controlSurfaceColor
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

        SettingsGroup {
            Layout.fillWidth: true
            title: qsTr("窗口背景")

            SettingsRow {
                title: qsTr("背景图片")
                description: qsTr("从背景库中选择，或导入并命名新的背景图片")
                controlWidth: 430

                RowLayout {
                    anchors.fill: parent
                    spacing: 8

                    HusSelect {
                        id: backgroundImageSelect

                        Layout.fillWidth: true
                        Layout.preferredHeight: 38
                        colorBg: root.controlSurfaceColor
                        clearEnabled: false
                        model: AppSettings.theme.backgroundImages
                        currentIndex: root.backgroundImageIndex
                        placeholderText: qsTr("选择背景图片")
                        contentDescription: qsTr("背景图片")
                        onActivated: {
                            if (currentValue === undefined || currentValue === null)
                                return;
                            AppSettings.theme.updateBackgroundImage(currentValue);
                            currentIndex = Qt.binding(() => root.backgroundImageIndex);
                        }
                    }

                    HusIconButton {
                        Layout.preferredWidth: 42
                        Layout.preferredHeight: 38
                        padding: 0
                        iconSource: HusIcon.UploadOutlined
                        iconSize: 17
                        contentDescription: qsTr("导入背景图片")
                        onClicked: backgroundImageDialog.open()
                    }

                    HusIconButton {
                        Layout.preferredWidth: 42
                        Layout.preferredHeight: 38
                        padding: 0
                        enabled: root.backgroundImageRemovable
                        iconSource: HusIcon.DeleteOutlined
                        iconSize: 17
                        colorIcon: enabled ? HusTheme.Primary.colorError
                                           : HusTheme.Primary.colorTextQuaternary
                        contentDescription: qsTr("删除当前背景")
                        onClicked: {
                            backgroundDeleteModal.imageSource = root.selectedBackgroundImageSource;
                            backgroundDeleteModal.imageName = root.selectedBackgroundImageName;
                            backgroundDeleteModal.deleteAttempted = false;
                            backgroundDeleteModal.open();
                        }
                    }
                }
            }

            SettingsRow {
                title: qsTr("表面不透明度")
                description: qsTr("调整主题表面对背景图片的覆盖程度")
                controlWidth: 430
                last: true

                RowLayout {
                    anchors.fill: parent
                    spacing: 12

                    HusSlider {
                        id: backgroundOpacitySlider

                        Layout.fillWidth: true
                        Layout.preferredHeight: 38
                        min: 0
                        max: 100
                        stepSize: 1
                        snapMode: HusSlider.SnapOnRelease
                        value: Math.round(AppSettings.theme.backgroundOpacity * 100)
                        contentDescription: qsTr("背景表面不透明度")
                        onFirstReleased: root.updateBackgroundOpacity(currentValue)
                    }

                    HusInputNumber {
                        id: backgroundOpacityInput

                        Layout.preferredWidth: 98
                        Layout.preferredHeight: 32
                        colorBg: root.controlSurfaceColor
                        min: 0
                        max: 100
                        step: 1
                        precision: 0
                        suffix: "%"
                        alwaysShowHandler: true
                        value: Math.round(AppSettings.theme.backgroundOpacity * 100)
                        Accessible.name: qsTr("背景表面不透明度百分比")
                        onValueModified: root.updateBackgroundOpacity(value)
                    }
                }
            }
        }
    }
}
