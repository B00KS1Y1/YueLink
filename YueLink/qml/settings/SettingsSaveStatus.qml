pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic

Item {
    id: root

    required property var settingsModel

    readonly property bool hasError: settingsModel !== null
                                     && settingsModel.saveState === SettingsModel.Error
    readonly property string statusText: hasError ? settingsModel.errorMessage : ""

    visible: statusText.length > 0
    implicitWidth: statusRow.implicitWidth
    implicitHeight: Math.max(24, statusRow.implicitHeight)

    Accessible.role: Accessible.StaticText
    Accessible.name: statusText

    RowLayout {
        id: statusRow

        anchors.fill: parent
        spacing: 6

        HusIconText {
            Layout.preferredWidth: 18
            iconSource: HusIcon.CloseCircleFilled
            iconSize: 16
            colorIcon: HusTheme.Primary.colorError
            Accessible.ignored: true
        }

        HusText {
            Layout.maximumWidth: 360
            text: root.statusText
            color: HusTheme.Primary.colorError
            font.pixelSize: Math.max(12, HusTheme.Primary.fontPrimarySize - 2)
            elide: Text.ElideRight
        }
    }
}
