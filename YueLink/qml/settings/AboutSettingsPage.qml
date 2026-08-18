pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import HuskarUI.Basic

Flickable {
    id: root

    readonly property string appVersion: Qt.application.version.length > 0
                                                 ? Qt.application.version : "0.1"
    readonly property url repositoryUrl: "https://github.com/B00KS1Y1/YueLink"
    readonly property url issuesUrl: "https://github.com/B00KS1Y1/YueLink/issues"
    readonly property var featureModel: [
        {
            "title": qsTr("自动发现"),
            "description": qsTr("自动发现同一局域网中的在线设备，无需手动配置服务器。"),
            "iconSource": HusIcon.WifiOutlined
        },
        {
            "title": qsTr("即时沟通"),
            "description": qsTr("专注清晰、直接的消息体验，让身边的沟通更轻松。"),
            "iconSource": HusIcon.MessageOutlined
        },
        {
            "title": qsTr("文件传输"),
            "description": qsTr("在设备之间直接发送图片与文件，并清楚掌握传输状态。"),
            "iconSource": HusIcon.SwapOutlined
        }
    ]
    readonly property var openSourceModel: [
        {
            "title": qsTr("Qt 6 / Qt Quick"),
            "description": qsTr("跨平台应用框架与声明式界面技术"),
            "meta": qsTr("Qt Project"),
            "iconSource": HusIcon.CodeOutlined,
            "projectUrl": "https://www.qt.io/"
        },
        {
            "title": qsTr("HuskarUI"),
            "description": qsTr("现代、统一的 Qt Quick 界面组件库"),
            "meta": qsTr("MIT License"),
            "iconSource": HusIcon.DesktopOutlined,
            "projectUrl": "https://github.com/mengps/HuskarUI"
        },
        {
            "title": qsTr("QsLog"),
            "description": qsTr("轻量、易用的 Qt 应用日志组件"),
            "meta": qsTr("BSD 3-Clause"),
            "iconSource": HusIcon.FileTextOutlined,
            "projectUrl": "https://github.com/victronenergy/QsLog"
        },
        {
            "title": qsTr("nlohmann/json"),
            "description": qsTr("用于现代 C++ 的 JSON 解析与序列化库"),
            "meta": qsTr("MIT License"),
            "iconSource": HusIcon.CodeOutlined,
            "projectUrl": "https://github.com/nlohmann/json"
        }
    ]

    contentWidth: width
    contentHeight: pageColumn.implicitHeight + 48
    boundsBehavior: Flickable.StopAtBounds
    clip: true
    ScrollBar.vertical: HusScrollBar { }

    function openExternalUrl(targetUrl: url): void {
        Qt.openUrlExternally(targetUrl);
    }

    component SectionHeader: RowLayout {
        id: sectionHeader

        required property string title

        spacing: 10

        Rectangle {
            Layout.preferredWidth: 4
            Layout.preferredHeight: 22
            radius: 2
            color: HusTheme.Primary.colorPrimary
            Accessible.ignored: true
        }

        HusText {
            Layout.fillWidth: true
            text: sectionHeader.title
            color: HusTheme.Primary.colorTextBase
            font.pixelSize: HusTheme.Primary.fontPrimarySizeHeading4
            font.weight: Font.DemiBold
        }
    }

    component FeatureCard: Rectangle {
        id: featureCard

        required property string title
        required property string description
        required property var iconSource

        implicitHeight: featureContent.implicitHeight + 36
        radius: HusTheme.Primary.radiusPrimary
        color: HusTheme.HusCard.colorBg
        border.width: 1
        border.color: HusTheme.Primary.colorSplit

        RowLayout {
            id: featureContent

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 18
            anchors.rightMargin: 18
            spacing: 14

            Rectangle {
                Layout.preferredWidth: 42
                Layout.preferredHeight: 42
                Layout.alignment: Qt.AlignTop
                radius: 12
                color: HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary,
                                               HusTheme.isDark ? 0.22 : 0.12)
                Accessible.ignored: true

                HusIconText {
                    anchors.centerIn: parent
                    iconSource: featureCard.iconSource
                    iconSize: 20
                    colorIcon: HusTheme.Primary.colorPrimary
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 5

                HusText {
                    Layout.fillWidth: true
                    text: featureCard.title
                    color: HusTheme.Primary.colorTextBase
                    font.pixelSize: HusTheme.Primary.fontPrimarySize
                    font.weight: Font.DemiBold
                }

                HusText {
                    Layout.fillWidth: true
                    text: featureCard.description
                    color: HusTheme.Primary.colorTextTertiary
                    font.pixelSize: Math.max(12, HusTheme.Primary.fontPrimarySize - 1)
                    wrapMode: Text.WordWrap
                }
            }
        }
    }

    component ProjectCard: Rectangle {
        id: projectCard

        required property string title
        required property string description
        required property string meta
        required property var iconSource
        required property url projectUrl

        implicitHeight: projectContent.implicitHeight + 32
        radius: HusTheme.Primary.radiusPrimary
        color: HusTheme.HusCard.colorBg
        border.width: 1
        border.color: HusTheme.Primary.colorSplit

        RowLayout {
            id: projectContent

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 18
            anchors.rightMargin: 12
            spacing: 14

            Rectangle {
                Layout.preferredWidth: 42
                Layout.preferredHeight: 42
                radius: 12
                color: HusTheme.Primary.colorFillSecondary
                Accessible.ignored: true

                HusIconText {
                    anchors.centerIn: parent
                    iconSource: projectCard.iconSource
                    iconSize: 19
                    colorIcon: HusTheme.Primary.colorTextTertiary
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 3

                HusText {
                    Layout.fillWidth: true
                    text: projectCard.title
                    color: HusTheme.Primary.colorTextBase
                    font.pixelSize: HusTheme.Primary.fontPrimarySize
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                }

                HusText {
                    Layout.fillWidth: true
                    text: projectCard.description
                    color: HusTheme.Primary.colorTextTertiary
                    font.pixelSize: Math.max(12, HusTheme.Primary.fontPrimarySize - 2)
                    elide: Text.ElideRight
                }

                HusText {
                    Layout.fillWidth: true
                    text: projectCard.meta
                    color: HusTheme.Primary.colorTextQuaternary
                    font.pixelSize: Math.max(12, HusTheme.Primary.fontPrimarySize - 2)
                    elide: Text.ElideRight
                }
            }

            HusIconButton {
                Layout.preferredWidth: 38
                Layout.preferredHeight: 38
                type: HusButton.Type_Text
                padding: 0
                iconSource: HusIcon.LinkOutlined
                iconSize: 17
                radiusBg.all: 9
                contentDescription: qsTr("打开 %1 项目主页").arg(projectCard.title)
                onClicked: root.openExternalUrl(projectCard.projectUrl)
            }
        }
    }

    ColumnLayout {
        id: pageColumn

        x: Math.max(24, (root.width - width) * 0.5)
        y: 8
        width: Math.min(900, root.width - 48)
        spacing: 20

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: Math.max(228, heroContent.implicitHeight + 52)
            radius: HusTheme.Primary.radiusPrimary * 1.5
            color: HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary,
                                           HusTheme.isDark ? 0.18 : 0.09)
            border.width: 1
            border.color: HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary,
                                                  HusTheme.isDark ? 0.34 : 0.18)

            RowLayout {
                id: heroContent

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: 30
                anchors.rightMargin: 30
                spacing: 26

                Rectangle {
                    Layout.preferredWidth: 112
                    Layout.preferredHeight: 112
                    Layout.alignment: Qt.AlignVCenter
                    radius: 28
                    color: HusTheme.HusCard.colorBg
                    border.width: 1
                    border.color: HusTheme.Primary.colorSplit

                    Image {
                        anchors.centerIn: parent
                        width: 78
                        height: 78
                        source: "qrc:/yuelink/assets/yuelink-app-icon.png"
                        sourceSize: Qt.size(78, 78)
                        fillMode: Image.PreserveAspectFit
                        Accessible.ignored: true
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    HusText {
                        Layout.fillWidth: true
                        text: qsTr("YueLink")
                        color: HusTheme.Primary.colorTextBase
                        font.pixelSize: HusTheme.Primary.fontPrimarySizeHeading2
                        font.weight: Font.Bold
                    }

                    HusText {
                        Layout.fillWidth: true
                        text: qsTr("让局域网沟通保持简单、直接")
                        color: HusTheme.Primary.colorTextBase
                        font.pixelSize: HusTheme.Primary.fontPrimarySizeHeading4
                        font.weight: Font.Medium
                        wrapMode: Text.WordWrap
                    }

                    HusText {
                        Layout.fillWidth: true
                        text: qsTr("YueLink 是一款基于 Qt 构建的局域网即时通信应用，支持设备发现、消息沟通以及图片和文件传输。")
                        color: HusTheme.Primary.colorTextTertiary
                        font.pixelSize: HusTheme.Primary.fontPrimarySize
                        wrapMode: Text.WordWrap
                    }

                    RowLayout {
                        spacing: 8

                        HusTag {
                            text: qsTr("版本 %1").arg(root.appVersion)
                            iconSource: HusIcon.RocketOutlined
                            presetColor: "blue"
                            Accessible.role: Accessible.StaticText
                            Accessible.name: text
                        }

                        HusTag {
                            text: qsTr("开源项目")
                            iconSource: HusIcon.GithubOutlined
                            presetColor: "green"
                            Accessible.role: Accessible.StaticText
                            Accessible.name: text
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: releaseHint.implicitHeight + 28
            radius: HusTheme.Primary.radiusPrimary
            color: HusThemeFunctions.alpha(HusTheme.Primary.colorWarning,
                                           HusTheme.isDark ? 0.16 : 0.09)
            border.width: 1
            border.color: HusThemeFunctions.alpha(HusTheme.Primary.colorWarning,
                                                  HusTheme.isDark ? 0.34 : 0.22)

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                spacing: 10

                HusIconText {
                    Layout.preferredWidth: 20
                    iconSource: HusIcon.InfoCircleOutlined
                    iconSize: 18
                    colorIcon: HusTheme.Primary.colorWarning
                    Accessible.ignored: true
                }

                HusText {
                    id: releaseHint

                    Layout.fillWidth: true
                    text: qsTr("当前仍是早期版本，功能与数据格式可能继续调整；传输重要文件时请保留原始副本。")
                    color: HusTheme.Primary.colorTextTertiary
                    font.pixelSize: HusTheme.Primary.fontPrimarySize
                    wrapMode: Text.WordWrap
                }
            }
        }

        SectionHeader {
            Layout.fillWidth: true
            title: qsTr("核心能力")
        }

        GridLayout {
            id: featureGrid

            Layout.fillWidth: true
            columns: width >= 780 ? 3 : width >= 520 ? 2 : 1
            columnSpacing: 12
            rowSpacing: 12

            Repeater {
                model: root.featureModel

                delegate: FeatureCard {
                    required property var modelData
                    required property int index

                    Layout.fillWidth: true
                    Layout.columnSpan: featureGrid.columns === 2 && index === 2 ? 2 : 1
                    Layout.minimumHeight: 122
                    title: modelData.title
                    description: modelData.description
                    iconSource: modelData.iconSource
                }
            }
        }

        SettingsGroup {
            Layout.fillWidth: true
            title: qsTr("版本与技术信息")
            surfaceColor: HusTheme.HusCard.colorBg

            SettingsRow {
                title: qsTr("应用版本")
                description: qsTr("当前安装的 YueLink 版本")
                controlWidth: 260

                HusText {
                    anchors.fill: parent
                    text: root.appVersion
                    color: HusTheme.Primary.colorTextBase
                    font.pixelSize: HusTheme.Primary.fontPrimarySize
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                }
            }

            SettingsRow {
                title: qsTr("界面技术")
                description: qsTr("跨平台界面与组件体系")
                controlWidth: 320

                HusText {
                    anchors.fill: parent
                    text: qsTr("Qt 6.8+ · Qt Quick · HuskarUI")
                    color: HusTheme.Primary.colorTextBase
                    font.pixelSize: HusTheme.Primary.fontPrimarySize
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
            }

            SettingsRow {
                title: qsTr("核心技术")
                description: qsTr("通信、存储与实现语言")
                controlWidth: 320

                HusText {
                    anchors.fill: parent
                    text: qsTr("C++17 · UDP/TCP · SQLite")
                    color: HusTheme.Primary.colorTextBase
                    font.pixelSize: HusTheme.Primary.fontPrimarySize
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
            }

            SettingsRow {
                title: qsTr("通信方式")
                description: qsTr("面向同一局域网中的设备")
                controlWidth: 320
                last: true

                HusText {
                    anchors.fill: parent
                    text: qsTr("设备直连 · 无需中心服务器")
                    color: HusTheme.Primary.colorTextBase
                    font.pixelSize: HusTheme.Primary.fontPrimarySize
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
            }
        }

        SectionHeader {
            Layout.fillWidth: true
            title: qsTr("开源项目与致谢")
        }

        GridLayout {
            Layout.fillWidth: true
            columns: width >= 700 ? 2 : 1
            columnSpacing: 12
            rowSpacing: 12

            Repeater {
                model: root.openSourceModel

                delegate: ProjectCard {
                    required property var modelData

                    Layout.fillWidth: true
                    Layout.minimumHeight: 112
                    title: modelData.title
                    description: modelData.description
                    meta: modelData.meta
                    iconSource: modelData.iconSource
                    projectUrl: modelData.projectUrl
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: developerContent.implicitHeight + 40
            radius: HusTheme.Primary.radiusPrimary
            color: HusTheme.HusCard.colorBg
            border.width: 1
            border.color: HusTheme.Primary.colorSplit

            ColumnLayout {
                id: developerContent

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                spacing: 14

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 14

                    Rectangle {
                        Layout.preferredWidth: 46
                        Layout.preferredHeight: 46
                        radius: 23
                        color: HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary,
                                                       HusTheme.isDark ? 0.22 : 0.12)
                        Accessible.ignored: true

                        HusIconText {
                            anchors.centerIn: parent
                            iconSource: HusIcon.UserOutlined
                            iconSize: 21
                            colorIcon: HusTheme.Primary.colorPrimary
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 3

                        HusText {
                            Layout.fillWidth: true
                            text: qsTr("开发者 · 系梨（XiL1）")
                            color: HusTheme.Primary.colorTextBase
                            font.pixelSize: HusTheme.Primary.fontPrimarySizeHeading4
                            font.weight: Font.DemiBold
                        }

                        HusText {
                            Layout.fillWidth: true
                            text: qsTr("欢迎通过项目仓库参与贡献、提出建议或反馈问题。")
                            color: HusTheme.Primary.colorTextTertiary
                            font.pixelSize: HusTheme.Primary.fontPrimarySize
                            wrapMode: Text.WordWrap
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    HusIconButton {
                        Layout.preferredHeight: 40
                        type: HusButton.Type_Primary
                        text: qsTr("查看源代码")
                        iconSource: HusIcon.GithubOutlined
                        iconSize: 17
                        contentDescription: qsTr("在浏览器中打开 YueLink 源代码仓库")
                        onClicked: root.openExternalUrl(root.repositoryUrl)
                    }

                    HusIconButton {
                        Layout.preferredHeight: 40
                        type: HusButton.Type_Default
                        text: qsTr("反馈问题")
                        iconSource: HusIcon.BugOutlined
                        iconSize: 17
                        contentDescription: qsTr("在浏览器中打开 YueLink 问题反馈页面")
                        onClicked: root.openExternalUrl(root.issuesUrl)
                    }

                    Item {
                        Layout.fillWidth: true
                    }
                }
            }
        }

        HusText {
            Layout.fillWidth: true
            text: qsTr("感谢每一位开源项目维护者与贡献者。第三方组件遵循各自的开源许可。")
            color: HusTheme.Primary.colorTextQuaternary
            font.pixelSize: Math.max(12, HusTheme.Primary.fontPrimarySize - 2)
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }
    }
}
