pragma Singleton

import QtQuick
import HuskarUI.Basic

QtObject {
    readonly property bool dark: HusTheme.isDark

    readonly property color canvas: dark ? "#0A0F1C" : "#F2F5FA"
    readonly property color navigationSurface: dark ? "#0E1524" : "#E9EEF6"
    readonly property color surface: dark ? "#111827" : "#FFFFFF"
    readonly property color surfaceSubtle: dark ? "#0D1422" : "#F7F9FC"
    readonly property color surfaceElevated: dark ? "#182234" : "#FFFFFF"
    readonly property color inputSurface: dark ? "#0B1220" : "#FFFFFF"

    readonly property color border: dark ? "#243044" : "#DCE3ED"
    readonly property color borderStrong: dark ? "#334159" : "#C8D2E0"
    readonly property color divider: dark ? "#202B3D" : "#E5EAF1"

    readonly property color textPrimary: dark ? "#F4F7FC" : "#182033"
    readonly property color textSecondary: dark ? "#AAB5C8" : "#526079"
    readonly property color textTertiary: dark ? "#758198" : "#647187"

    readonly property color accent: HusTheme.Primary.colorPrimary
    readonly property color onAccent: {
        const value = Qt.color(accent);
        const luminance = value.r * 0.299 + value.g * 0.587 + value.b * 0.114;
        return luminance > 0.58 ? "#101522" : "#FFFFFF";
    }
    readonly property color accentSoft: HusThemeFunctions.alpha(accent,
                                                                 dark ? 0.18 : 0.1)
    readonly property color accentSoftStrong: HusThemeFunctions.alpha(accent,
                                                                       dark ? 0.28 : 0.16)
    readonly property color hover: dark ? "#182235" : "#E9EEF6"
    readonly property color success: dark ? "#48D6A1" : "#14845D"
    readonly property color error: dark ? "#FF7B86" : "#C73545"

    readonly property int radiusSmall: 8
    readonly property int radiusMedium: 12
    readonly property int radiusLarge: 16
}
