pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic
import HuskarUI.Basic

Item {
    id: root

    property var selectedMembers: ({})
    property int selectedCount: 0
    property string groupName: ""
    readonly property int minimumMembers: 2
    readonly property int maximumMembers: 31
    readonly property bool canCreate: groupName.trim().length > 0
                                      && selectedCount >= minimumMembers
                                      && selectedCount <= maximumMembers

    signal closed()
    signal groupCreated(string conversationId)

    function isSelected(peerId: string): bool {
        return selectedMembers[peerId] === true;
    }

    function setSelected(peerId: string, selected: bool): void {
        const next = Object.assign({}, selectedMembers);
        if (selected)
            next[peerId] = true;
        else
            delete next[peerId];
        selectedMembers = next;
        selectedCount = Object.keys(next).length;
    }

    function toggleMember(peerId: string): void {
        if (!isSelected(peerId) && selectedCount >= maximumMembers)
            return;
        setSelected(peerId, !isSelected(peerId));
    }

    function createGroup(): void {
        if (!canCreate)
            return;
        const conversationId = LanChat.createGroup(groupName.trim(),
                                                   Object.keys(selectedMembers));
        if (conversationId.length === 0)
            return;
        groupCreated(conversationId);
        createModal.close();
    }

    HusModal {
        id: createModal

        width: Math.min(560, root.width - 64)
        height: Math.min(650, root.height - 32)
        closable: false
        maskClosable: true
        contentDelegate: Item {
            height: createModal.height

            HusText {
                id: titleText

                anchors.left: parent.left
                anchors.top: parent.top
                anchors.leftMargin: 24
                anchors.topMargin: 20
                text: qsTr("创建群聊")
                color: HusTheme.Primary.colorTextBase
                font.pixelSize: HusTheme.Primary.fontPrimarySizeHeading4
                font.weight: Font.Medium
            }

            HusIconButton {
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.rightMargin: 12
                anchors.topMargin: 12
                width: 34
                height: 34
                padding: 0
                type: HusButton.Type_Text
                iconSource: HusIcon.CloseOutlined
                iconSize: 18
                contentDescription: qsTr("关闭创建群聊窗口")
                onClicked: createModal.close()
            }

            HusText {
                id: nameLabel

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: titleText.bottom
                anchors.leftMargin: 24
                anchors.rightMargin: 24
                anchors.topMargin: 22
                text: qsTr("群名称")
                color: HusTheme.Primary.colorTextTertiary
                font.pixelSize: HusTheme.Primary.fontPrimarySize
                font.weight: Font.Medium
            }

            HusInput {
                id: groupNameInput

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: nameLabel.bottom
                anchors.leftMargin: 24
                anchors.rightMargin: 24
                anchors.topMargin: 8
                height: 40
                maximumLength: 64
                clearEnabled: "active"
                type: HusInput.Type_Filled
                placeholderText: qsTr("输入 1–64 个字符")
                contentDescription: qsTr("群名称")
                text: root.groupName
                onTextChanged: root.groupName = text
                Keys.onReturnPressed: root.createGroup()
                Component.onCompleted: Qt.callLater(() => forceActiveFocus())
            }

            Row {
                id: memberHeader

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: groupNameInput.bottom
                anchors.leftMargin: 24
                anchors.rightMargin: 24
                anchors.topMargin: 20

                HusText {
                    text: qsTr("选择联系人")
                    color: HusTheme.Primary.colorTextTertiary
                    font.pixelSize: HusTheme.Primary.fontPrimarySize
                    font.weight: Font.Medium
                }

                HusText {
                    width: parent.width - x
                    text: qsTr("已选 %1/%2").arg(root.selectedCount)
                                              .arg(root.maximumMembers)
                    color: root.selectedCount >= root.minimumMembers
                           ? HusTheme.Primary.colorTextQuaternary
                           : HusTheme.Primary.colorWarning
                    horizontalAlignment: Text.AlignRight
                    font.pixelSize: HusTheme.Primary.fontPrimarySize
                }
            }

            Rectangle {
                id: memberSurface

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: memberHeader.bottom
                anchors.bottom: footer.top
                anchors.leftMargin: 24
                anchors.rightMargin: 24
                anchors.topMargin: 8
                anchors.bottomMargin: 14
                radius: HusTheme.Primary.radiusPrimary
                color: HusTheme.Primary.colorFillTertiary
                border.width: 1
                border.color: HusTheme.Primary.colorSplit

                ListView {
                    id: memberList

                    anchors.fill: parent
                    anchors.margins: 6
                    model: LanChat.peers
                    boundsBehavior: Flickable.StopAtBounds
                    clip: true
                    ScrollBar.vertical: HusScrollBar { }

                    delegate: Item {
                        id: memberDelegate

                        required property string peerId
                        required property string title
                        required property string initial
                        required property color avatarColor
                        required property bool online

                        width: memberList.width
                        height: 58

                        readonly property bool selected: root.isSelected(peerId)
                        readonly property bool selectable: selected
                                                           || root.selectedCount
                                                              < root.maximumMembers

                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: 2
                            radius: HusTheme.Primary.radiusPrimary
                            color: memberMouse.containsMouse
                                   ? HusTheme.Primary.colorFillSecondary
                                   : "transparent"
                            border.width: memberMouse.activeFocus ? 1 : 0
                            border.color: HusTheme.Primary.colorPrimary
                        }

                        Rectangle {
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            anchors.verticalCenter: parent.verticalCenter
                            width: 22
                            height: 22
                            radius: 6
                            color: memberDelegate.selected
                                   ? HusTheme.Primary.colorPrimary
                                   : "transparent"
                            border.width: 1
                            border.color: memberDelegate.selected
                                          ? HusTheme.Primary.colorPrimary
                                          : HusTheme.Primary.colorTextQuaternary

                            HusIconText {
                                anchors.centerIn: parent
                                visible: memberDelegate.selected
                                iconSource: HusIcon.CheckOutlined
                                iconSize: 14
                                colorIcon: "white"
                            }
                        }

                        HusAvatar {
                            id: memberAvatar

                            anchors.left: parent.left
                            anchors.leftMargin: 44
                            anchors.verticalCenter: parent.verticalCenter
                            size: 38
                            textSource: memberDelegate.initial
                            colorBg: memberDelegate.avatarColor
                            textSize: HusAvatar.Size_Auto

                            HusBadge {
                                dot: true
                                badgeState: memberDelegate.online
                                            ? HusBadge.State_Success
                                            : HusBadge.State_Default
                            }
                        }

                        HusText {
                            anchors.left: memberAvatar.right
                            anchors.right: parent.right
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            anchors.verticalCenter: parent.verticalCenter
                            text: memberDelegate.title
                            color: memberDelegate.selectable
                                   ? HusTheme.Primary.colorTextBase
                                   : HusTheme.Primary.colorTextDisabled
                            elide: Text.ElideRight
                            font.pixelSize: HusTheme.Primary.fontPrimarySize
                            font.weight: Font.Medium
                        }

                        MouseArea {
                            id: memberMouse

                            anchors.fill: parent
                            activeFocusOnTab: true
                            enabled: memberDelegate.selectable
                            hoverEnabled: true
                            cursorShape: enabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
                            Accessible.role: Accessible.CheckBox
                            Accessible.name: memberDelegate.title
                            Accessible.checked: memberDelegate.selected
                            onClicked: root.toggleMember(memberDelegate.peerId)
                            Keys.onReturnPressed: root.toggleMember(memberDelegate.peerId)
                            Keys.onSpacePressed: root.toggleMember(memberDelegate.peerId)
                        }
                    }

                    Column {
                        anchors.centerIn: parent
                        spacing: 8
                        visible: memberList.count === 0

                        HusText {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: qsTr("暂无联系人")
                            color: HusTheme.Primary.colorTextBase
                            font.pixelSize: HusTheme.Primary.fontPrimarySizeHeading5
                        }

                        HusText {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: qsTr("先启动局域网发现并等待联系人出现")
                            color: HusTheme.Primary.colorTextQuaternary
                            font.pixelSize: HusTheme.Primary.fontPrimarySize
                        }
                    }
                }
            }

            Row {
                id: footer

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.leftMargin: 24
                anchors.rightMargin: 24
                anchors.bottomMargin: 20
                height: 40
                spacing: 10

                HusText {
                    width: parent.width - cancelButton.width
                           - createButton.width - parent.spacing * 2
                    anchors.verticalCenter: parent.verticalCenter
                    text: root.selectedCount < root.minimumMembers
                          ? qsTr("至少选择 2 位联系人")
                          : qsTr("创建后会立即同步给在线成员")
                    color: root.selectedCount < root.minimumMembers
                           ? HusTheme.Primary.colorWarning
                           : HusTheme.Primary.colorTextQuaternary
                    elide: Text.ElideRight
                    font.pixelSize: HusTheme.Primary.fontPrimarySize
                }

                HusButton {
                    id: cancelButton

                    height: 40
                    text: qsTr("取消")
                    onClicked: createModal.close()
                }

                HusButton {
                    id: createButton

                    height: 40
                    type: HusButton.Type_Primary
                    text: qsTr("创建群聊")
                    enabled: root.canCreate
                    onClicked: root.createGroup()
                }
            }
        }

        Component.onCompleted: open()
        onClosed: root.closed()
    }

    Shortcut {
        sequence: "Escape"
        onActivated: createModal.close()
    }
}
