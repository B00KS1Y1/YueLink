pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Dialogs
import QtQuick.Layouts
import HuskarUI.Basic

Item {
    id: root

    property int currentCategoryIndex: 0
    property string errorText: ""

    property string draftThemeMode: "system"
    property string draftPrimaryColor: "#4F7CFF"
    property bool draftAnimationsEnabled: true
    property bool draftNotificationsEnabled: true
    property string draftDownloadDirectory: ""
    property string draftLogLevel: "info"
    property string draftLogFilePath: ""
    property bool draftSourceLocationEnabled: false
    property bool draftSeparateThreadEnabled: true

    readonly property bool compactNavigation: width < 760
    readonly property int navigationWidth: compactNavigation ? 76 : 204
    readonly property string currentCategoryKey: categoryModel[currentCategoryIndex].key
    readonly property string currentCategoryTitle: categoryModel[currentCategoryIndex].label
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
        }
    ]
    readonly property var themeColorPresets: [
        "#4F7CFF",
        "#16A6B6",
        "#22A06B",
        "#E58A1F",
        "#8B5CF6"
    ]
    readonly property bool settingsValid: ["system", "light", "dark"].indexOf(draftThemeMode) >= 0
                                                  && ["trace", "debug", "info", "warn", "error", "critical", "off"].indexOf(draftLogLevel) >= 0
    readonly property bool hasUnsavedChanges: draftThemeMode !== AppSettings.themeMode
                                                  || draftPrimaryColor.toUpperCase() !== AppSettings.primaryColor.toUpperCase()
                                                  || draftAnimationsEnabled !== AppSettings.animationsEnabled
                                                  || draftNotificationsEnabled !== AppSettings.notificationsEnabled
                                                  || draftDownloadDirectory !== AppSettings.downloadDirectory
                                                  || draftLogLevel !== AppSettings.logLevel
                                                  || draftLogFilePath !== AppSettings.logFilePath
                                                  || draftSourceLocationEnabled !== AppSettings.sourceLocationEnabled
                                                  || draftSeparateThreadEnabled !== AppSettings.separateThreadEnabled

    signal closeRequested()
    signal saved()

    function loadSettings(): void {
        draftThemeMode = AppSettings.themeMode;
        draftPrimaryColor = AppSettings.primaryColor;
        draftAnimationsEnabled = AppSettings.animationsEnabled;
        draftNotificationsEnabled = AppSettings.notificationsEnabled;
        draftDownloadDirectory = AppSettings.downloadDirectory;
        draftLogLevel = AppSettings.logLevel;
        draftLogFilePath = AppSettings.logFilePath;
        draftSourceLocationEnabled = AppSettings.sourceLocationEnabled;
        draftSeparateThreadEnabled = AppSettings.separateThreadEnabled;
        currentCategoryIndex = 0;
        errorText = "";
    }

    function saveSettings(): void {
        if (AppSettings.save(draftThemeMode,
                             draftPrimaryColor,
                             draftAnimationsEnabled,
                             draftNotificationsEnabled,
                             draftDownloadDirectory,
                             draftLogLevel,
                             draftLogFilePath,
                             draftSourceLocationEnabled,
                             draftSeparateThreadEnabled)) {
            errorText = "";
            saved();
        } else {
            errorText = AppSettings.lastError;
        }
    }

    function requestClose(): void {
        if (hasUnsavedChanges)
            discardChangesModal.openWarning();
        else
            closeRequested();
    }

    function localFilePath(fileUrl: url): string {
        const fileUrlText = decodeURIComponent(fileUrl.toString());
        if (fileUrlText.startsWith("file:///"))
            return Qt.platform.os === "windows" ? fileUrlText.substring(8) : fileUrlText.substring(7);
        if (fileUrlText.startsWith("file://"))
            return "//" + fileUrlText.substring(7);
        return "";
    }

    function encodedPath(filePath: string): string {
        return encodeURIComponent(filePath).replace(/%2F/gi, "/");
    }

    function localFileUrl(filePath: string): url {
        const normalizedPath = filePath.trim().replace(/\\/g, "/");
        if (normalizedPath.startsWith("//"))
            return "file://" + root.encodedPath(normalizedPath.substring(2));
        if (/^[A-Za-z]:\//.test(normalizedPath))
            return "file:///" + normalizedPath.substring(0, 2)
                    + root.encodedPath(normalizedPath.substring(2));
        if (normalizedPath.startsWith("/"))
            return "file://" + root.encodedPath(normalizedPath);
        return "";
    }

    function containingDirectoryPath(filePath: string): string {
        const normalizedPath = filePath.trim().replace(/\\/g, "/");
        const separatorIndex = normalizedPath.lastIndexOf("/");
        if (/^[A-Za-z]:\//.test(normalizedPath) && separatorIndex === 2)
            return normalizedPath.substring(0, 3);
        if (separatorIndex === 0)
            return "/";
        return separatorIndex > 0 ? normalizedPath.substring(0, separatorIndex) : "";
    }

    function openLogFileDialog(): void {
        const currentDirectoryUrl = root.localFileUrl(
                                      root.containingDirectoryPath(draftLogFilePath));
        if (currentDirectoryUrl.toString().length > 0)
            logFileDialog.currentFolder = currentDirectoryUrl;
        logFileDialog.open();
    }

    function openDownloadDirectoryDialog(): void {
        const currentDirectoryUrl = root.localFileUrl(draftDownloadDirectory);
        if (currentDirectoryUrl.toString().length > 0)
            downloadDirectoryDialog.currentFolder = currentDirectoryUrl;
        downloadDirectoryDialog.open();
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
        colorBg: selected ? AppTheme.accent
                          : hovered ? AppTheme.hover : "transparent"
        colorText: selected ? AppTheme.onAccent : AppTheme.textSecondary
        colorIcon: colorText
        colorBorder: visualFocus ? AppTheme.accent : "transparent"
        borderWidth: visualFocus ? 2 : 0
        radiusBg.all: 10
        contentDescription: label

        Accessible.role: Accessible.RadioButton
        Accessible.name: label
        Accessible.checked: selected
    }

    component SettingsGroup: ColumnLayout {
        id: settingsGroup

        required property string title
        default property alias content: settingsGroupContent.data

        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Rectangle {
                Layout.preferredWidth: 4
                Layout.preferredHeight: 22
                radius: 2
                color: AppTheme.accent
                Accessible.ignored: true
            }

            HusText {
                Layout.fillWidth: true
                text: settingsGroup.title
                color: AppTheme.textPrimary
                font.pixelSize: HusTheme.Primary.fontPrimarySizeHeading4
                font.weight: Font.DemiBold
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: settingsGroupContent.implicitHeight
            radius: AppTheme.radiusLarge
            color: AppTheme.surface
            border.width: 1
            border.color: AppTheme.border

            ColumnLayout {
                id: settingsGroupContent

                anchors.fill: parent
                spacing: 0
            }
        }
    }

    component SettingsRow: Item {
        id: settingsRow

        required property string title
        property string description: ""
        property int controlWidth: 280
        property bool last: false
        default property alias control: settingsControlHost.data

        Layout.fillWidth: true
        Layout.preferredHeight: description.length > 0 ? 76 : 62

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 20
            anchors.rightMargin: 20
            spacing: 24

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 3

                HusText {
                    Layout.fillWidth: true
                    text: settingsRow.title
                    color: AppTheme.textPrimary
                    font.pixelSize: HusTheme.Primary.fontPrimarySize
                    font.weight: Font.Medium
                    elide: Text.ElideRight
                }

                HusText {
                    Layout.fillWidth: true
                    visible: settingsRow.description.length > 0
                    text: settingsRow.description
                    color: AppTheme.textTertiary
                    font.pixelSize: Math.max(12, HusTheme.Primary.fontPrimarySize - 2)
                    elide: Text.ElideRight
                }
            }

            Item {
                id: settingsControlHost

                Layout.preferredWidth: Math.min(settingsRow.controlWidth,
                                                Math.max(180, settingsRow.width * 0.52))
                Layout.preferredHeight: 40
            }
        }

        Rectangle {
            anchors.left: settingsRow.left
            anchors.right: settingsRow.right
            anchors.bottom: settingsRow.bottom
            anchors.leftMargin: 20
            anchors.rightMargin: 20
            height: 1
            visible: !settingsRow.last
            color: AppTheme.divider
            Accessible.ignored: true
        }
    }

    component ThemeColorSwatch: HusButton {
        id: themeColorSwatch

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
        colorBorder: selected ? AppTheme.textPrimary : AppTheme.borderStrong
        borderWidth: selected ? 2 : 1
        radiusBg.all: 9
        contentDescription: qsTr("选择主题色 %1").arg(String(swatchColor).toUpperCase())

        Accessible.role: Accessible.RadioButton
        Accessible.name: contentDescription
        Accessible.checked: selected

        HusIconText {
            anchors.centerIn: parent
            visible: themeColorSwatch.selected
            iconSource: HusIcon.CheckOutlined
            iconSize: 17
            colorIcon: themeColorSwatch.checkColor
            Accessible.ignored: true
        }
    }

    Component.onCompleted: root.loadSettings()
    onCurrentCategoryIndexChanged: settingsFlickable.contentY = 0

    Shortcut {
        sequence: StandardKey.Cancel
        onActivated: root.requestClose()
    }

    FileDialog {
        id: logFileDialog

        title: qsTr("选择日志文件")
        fileMode: FileDialog.SaveFile
        defaultSuffix: "log"
        nameFilters: [qsTr("日志文件 (*.log)"), qsTr("所有文件 (*)")]
        onAccepted: root.draftLogFilePath = root.localFilePath(selectedFile)
    }

    FolderDialog {
        id: downloadDirectoryDialog

        title: qsTr("选择下载目录")
        onAccepted: root.draftDownloadDirectory = root.localFilePath(selectedFolder)
    }

    Rectangle {
        anchors.fill: parent
        color: AppTheme.canvas
        Accessible.ignored: true
    }

    Rectangle {
        id: navigationPanel

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: root.navigationWidth
        color: AppTheme.navigationSurface
        border.width: 1
        border.color: AppTheme.border

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
                    color: AppTheme.textPrimary
                    font.pixelSize: HusTheme.Primary.fontPrimarySize
                    font.weight: Font.Medium
                    elide: Text.ElideRight
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: AppTheme.divider
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
                        onClicked: root.currentCategoryIndex = index
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
                color: AppTheme.textTertiary
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
        color: AppTheme.canvas

        Item {
            id: pageHeader

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: 94

            Column {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: 32
                anchors.rightMargin: 32
                spacing: 5

                HusText {
                    width: parent.width
                    text: root.currentCategoryTitle
                    color: AppTheme.textPrimary
                    font.pixelSize: HusTheme.Primary.fontPrimarySizeHeading2
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                }

                HusText {
                    width: parent.width
                    text: root.currentCategoryDescription
                    color: AppTheme.textSecondary
                    font.pixelSize: HusTheme.Primary.fontPrimarySize
                    elide: Text.ElideRight
                }
            }
        }

        Flickable {
            id: settingsFlickable

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: pageHeader.bottom
            anchors.bottom: actionFooter.top
            contentWidth: width
            contentHeight: pageColumn.implicitHeight + 48
            boundsBehavior: Flickable.StopAtBounds
            clip: true
            ScrollBar.vertical: HusScrollBar { }

            ColumnLayout {
                id: pageColumn

                x: Math.max(24, (settingsFlickable.width - width) * 0.5)
                y: 8
                width: Math.min(900, settingsFlickable.width - 48)
                spacing: 20

                ColumnLayout {
                    Layout.fillWidth: true
                    visible: root.currentCategoryKey === "general"
                    spacing: 20

                    SettingsGroup {
                        Layout.fillWidth: true
                        title: qsTr("消息通知")

                        SettingsRow {
                            title: qsTr("系统通知")
                            description: qsTr("收到新消息时显示桌面通知")
                            controlWidth: 64
                            last: true

                            HusSwitch {
                                anchors.centerIn: parent
                                width: 48
                                height: 28
                                checked: root.draftNotificationsEnabled
                                contentDescription: qsTr("启用系统通知")
                                onToggled: root.draftNotificationsEnabled = checked
                            }
                        }
                    }

                    SettingsGroup {
                        Layout.fillWidth: true
                        title: qsTr("文件接收")

                        SettingsRow {
                            title: qsTr("下载目录")
                            description: qsTr("接收到的文件将保存到此位置")
                            controlWidth: 430
                            last: true

                            RowLayout {
                                anchors.fill: parent
                                spacing: 8

                                HusInput {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 38
                                    text: root.draftDownloadDirectory
                                    selectByMouse: true
                                    verticalAlignment: TextInput.AlignVCenter
                                    contentDescription: qsTr("下载目录")
                                    onTextChanged: root.draftDownloadDirectory = text
                                }

                                HusIconButton {
                                    Layout.preferredWidth: 42
                                    Layout.preferredHeight: 38
                                    padding: 0
                                    iconSource: HusIcon.FolderOpenOutlined
                                    iconSize: 17
                                    contentDescription: qsTr("选择下载目录")
                                    onClicked: root.openDownloadDirectoryDialog()
                                }
                            }
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    visible: root.currentCategoryKey === "appearance"
                    spacing: 20

                    SettingsGroup {
                        Layout.fillWidth: true
                        title: qsTr("主题")

                        SettingsRow {
                            title: qsTr("主题模式")
                            description: qsTr("选择浅色、深色或跟随系统")
                            controlWidth: 360

                            HusSegmented {
                                anchors.fill: parent
                                block: true
                                currentIndex: root.draftThemeMode === "system" ? 0
                                              : root.draftThemeMode === "light" ? 1 : 2
                                options: [
                                    { "label": qsTr("跟随系统"), "value": "system" },
                                    { "label": qsTr("浅色"), "value": "light" },
                                    { "label": qsTr("深色"), "value": "dark" }
                                ]
                                onCurrentValueChanged: {
                                    if (currentValue !== undefined && currentValue !== null)
                                        root.draftThemeMode = currentValue;
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
                                    model: root.themeColorPresets

                                    delegate: ThemeColorSwatch {
                                        required property var modelData

                                        Layout.preferredWidth: 36
                                        Layout.preferredHeight: 36
                                        swatchColor: modelData
                                        selected: String(modelData).toUpperCase()
                                                  === root.draftPrimaryColor.toUpperCase()
                                        onClicked: root.draftPrimaryColor = String(modelData).toUpperCase()
                                    }
                                }

                                HusColorPicker {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 104
                                    Layout.preferredHeight: 38
                                    autoChange: false
                                    changeValue: root.draftPrimaryColor
                                    defaultValue: root.draftPrimaryColor
                                    showText: true
                                    alphaEnabled: false
                                    format: "hex"
                                    title: qsTr("自定义主题色")
                                    Accessible.name: qsTr("自定义主题色")
                                    onChange: color => root.draftPrimaryColor = toHexString(color)
                                }
                            }
                        }

                        SettingsRow {
                            title: qsTr("界面动画")
                            description: qsTr("显示页面切换和控件反馈动画")
                            controlWidth: 64
                            last: true

                            HusSwitch {
                                anchors.centerIn: parent
                                width: 48
                                height: 28
                                checked: root.draftAnimationsEnabled
                                contentDescription: qsTr("启用界面动画")
                                onToggled: root.draftAnimationsEnabled = checked
                            }
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    visible: root.currentCategoryKey === "advanced"
                    spacing: 20

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 54
                        radius: AppTheme.radiusMedium
                        color: AppTheme.accentSoft
                        border.width: 1
                        border.color: AppTheme.accentSoftStrong

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 16
                            anchors.rightMargin: 16
                            spacing: 10

                            HusIconText {
                                Layout.preferredWidth: 20
                                iconSource: HusIcon.InfoCircleOutlined
                                iconSize: 18
                                colorIcon: AppTheme.accent
                                Accessible.ignored: true
                            }

                            HusText {
                                Layout.fillWidth: true
                                text: qsTr("这些选项主要用于问题诊断，一般情况下无需修改。")
                                color: AppTheme.textSecondary
                                font.pixelSize: HusTheme.Primary.fontPrimarySize
                                wrapMode: Text.WordWrap
                            }
                        }
                    }

                    SettingsGroup {
                        Layout.fillWidth: true
                        title: qsTr("诊断日志")

                        SettingsRow {
                            title: qsTr("日志级别")
                            description: qsTr("级别越低，记录的诊断信息越详细")
                            controlWidth: 180

                            HusSelect {
                                anchors.fill: parent
                                clearEnabled: false
                                currentIndex: root.draftLogLevel === "trace" ? 0
                                              : root.draftLogLevel === "debug" ? 1
                                              : root.draftLogLevel === "info" ? 2
                                              : root.draftLogLevel === "warn" ? 3
                                              : root.draftLogLevel === "error" ? 4
                                              : root.draftLogLevel === "critical" ? 5 : 6
                                contentDescription: qsTr("日志级别")
                                model: [
                                    { "label": qsTr("Trace"), "value": "trace" },
                                    { "label": qsTr("Debug"), "value": "debug" },
                                    { "label": qsTr("Info"), "value": "info" },
                                    { "label": qsTr("Warning"), "value": "warn" },
                                    { "label": qsTr("Error"), "value": "error" },
                                    { "label": qsTr("Critical"), "value": "critical" },
                                    { "label": qsTr("关闭"), "value": "off" }
                                ]
                                onCurrentValueChanged: {
                                    if (currentValue !== undefined && currentValue !== null)
                                        root.draftLogLevel = currentValue;
                                }
                            }
                        }

                        SettingsRow {
                            title: qsTr("日志文件路径")
                            description: qsTr("诊断日志保存到此文件")
                            controlWidth: 430

                            RowLayout {
                                anchors.fill: parent
                                spacing: 8

                                HusInput {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 38
                                    text: root.draftLogFilePath
                                    selectByMouse: true
                                    verticalAlignment: TextInput.AlignVCenter
                                    contentDescription: qsTr("日志文件路径")
                                    onTextChanged: root.draftLogFilePath = text
                                }

                                HusIconButton {
                                    Layout.preferredWidth: 42
                                    Layout.preferredHeight: 38
                                    padding: 0
                                    iconSource: HusIcon.FolderOpenOutlined
                                    iconSize: 17
                                    contentDescription: qsTr("选择日志文件路径")
                                    onClicked: root.openLogFileDialog()
                                }
                            }
                        }

                        SettingsRow {
                            title: qsTr("记录源码位置")
                            description: qsTr("在日志中附带源码文件路径和行号")
                            controlWidth: 64

                            HusSwitch {
                                anchors.centerIn: parent
                                width: 48
                                height: 28
                                checked: root.draftSourceLocationEnabled
                                contentDescription: qsTr("在日志中记录源码位置")
                                onToggled: root.draftSourceLocationEnabled = checked
                            }
                        }

                        SettingsRow {
                            title: qsTr("独立线程写入")
                            description: qsTr("通过专用线程异步写入日志文件")
                            controlWidth: 64
                            last: true

                            HusSwitch {
                                anchors.centerIn: parent
                                width: 48
                                height: 28
                                checked: root.draftSeparateThreadEnabled
                                contentDescription: qsTr("通过独立线程异步写入日志")
                                onToggled: root.draftSeparateThreadEnabled = checked
                            }
                        }
                    }
                }

            }
        }

        Rectangle {
            id: actionFooter

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 72
            color: AppTheme.surface

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                height: 1
                color: AppTheme.border
                Accessible.ignored: true
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 24
                anchors.rightMargin: 24
                spacing: 10

                HusIconText {
                    Layout.preferredWidth: visible ? 18 : 0
                    visible: root.errorText.length > 0
                    iconSource: HusIcon.CloseCircleFilled
                    iconSize: 16
                    colorIcon: AppTheme.error
                    Accessible.ignored: true
                }

                HusText {
                    Layout.fillWidth: true
                    visible: text.length > 0
                    text: root.errorText.length > 0
                          ? root.errorText
                          : root.hasUnsavedChanges ? qsTr("有未保存的更改") : ""
                    color: root.errorText.length > 0
                           ? AppTheme.error : AppTheme.textTertiary
                    font.pixelSize: HusTheme.Primary.fontPrimarySize
                    elide: Text.ElideRight
                }

                HusButton {
                    Layout.preferredWidth: 92
                    Layout.preferredHeight: 40
                    text: qsTr("取消")
                    contentDescription: qsTr("取消设置修改")
                    onClicked: root.requestClose()
                }

                HusButton {
                    Layout.preferredWidth: 112
                    Layout.preferredHeight: 40
                    type: HusButton.Type_Primary
                    text: qsTr("保存设置")
                    enabled: root.hasUnsavedChanges && root.settingsValid
                    contentDescription: qsTr("保存应用设置")
                    onClicked: root.saveSettings()
                }
            }
        }
    }

    HusModal {
        id: discardChangesModal

        width: 440
        title: qsTr("放弃未保存的更改？")
        description: qsTr("离开设置页后，本次修改将不会保存。")
        confirmText: qsTr("放弃更改")
        cancelText: qsTr("继续编辑")
        closable: false
        maskClosable: false
        onConfirm: {
            discardChangesModal.close();
            root.closeRequested();
        }
        onCancel: discardChangesModal.close()
    }
}
