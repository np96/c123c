import QtQuick 2.7
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.0
import cpp.chat.qml 1.0
Page {
    property SqlGroupModel groupModel : window.groupModel
    property SqlContactModel contactModel : window.contactModel
    onVisibleChanged: {
            if (visible) {
            window.contactModel.contact = ""
        }
    }
    header: ToolBar {
        height: 40
        Label {
            anchors.verticalCenter: parent.verticalCenter
            id: groupsHeader
            text: qsTr("Groups")
            font.pixelSize: 17
            anchors.left: parent.left
        }
        Button {
            text: qsTr("+")
            onClicked: serverConnect.addGroupRequest()
            id: newGroupButton
            flat: true
            anchors.left: groupsHeader.right
            width: 50
        }
    }

    id: groupsView
    ColumnLayout {
        anchors.fill: parent
        ListView {
            id: lView
            Layout.fillHeight: true
            Layout.fillWidth: true
            leftMargin: 5
            clip: false
            verticalLayoutDirection: ListView.TopToBottom
            spacing: 0
            model: contactModel
            delegate: ItemDelegate {
                id: deleg
                Rectangle {
                    id: rec
                    anchors.fill: parent
                    clip: true
                    Text {
                        text: model.contactid
                        anchors.bottom: parent.bottom
                        width: 160
                        height: 20
                        font.pixelSize: 17
                    }
                }
                Rectangle { // bottom border
                        anchors.bottom: rec.bottom
                        height: 0.5
                        width: parent.width
                        color: "lightgrey"
                }
                Rectangle { // right border
                        anchors.bottom: rec.bottom
                        anchors.right: rec.right
                        height: parent.height
                        width: 0.5
                        color: "lightgrey"
                }
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons :Qt.LeftButton | Qt.RightButton
                    onClicked: {
                        stckView.pop()
                        if (mouse.button === Qt.LeftButton) {
                            groupModel.group = model.contactid
                            stckView.push(["qrc:/MessangesView.qml", {group: model.contactid, mdl: groupsView.groupModel}],
                                                            StackView.Immediate)
                        } else {
                            stckView.push(["qrc:/ContactPreferences.qml", {contactId: model.contactid, mdl: contactModel}],
                                                            StackView.Immediate)
                        }
                    }
                }
            }
        }
    }
}
