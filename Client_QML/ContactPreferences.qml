import QtQuick 2.7
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.0
import cpp.chat.qml 1.0
Page {
    id: pg
    property SqlContactModel mdl
    property string contactId

    header: ToolBar {
        id: toolbar
        height: 40
        Layout.fillWidth: true
        ToolButton {
            id: backButton
            text: qsTr("Back")
            anchors.left: parent.left
            anchors.leftMargin: 0
            anchors.verticalCenter: parent.verticalCenter
            onClicked: {
                pg.StackView.view.pop()
                //pg.StackView.view.push(["qrc:/GroupList.qml"], StackView.replace, StackView.Immediate)
            }
        }
        Label {
            //anchors.verticalCenter: backButton.verticalCenter
            id: msgListTitle
            text: contactId
            font.pixelSize: 20
            anchors.centerIn: parent
        }
    }

    ColumnLayout {
        anchors.fill: parent
        id: preferencesParent
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 20
            clip: true
            interactive: false
            verticalLayoutDirection: ListView.TopToBottom
            id: editView
            model: SqlContactModel {contact: contactId}
            delegate: ItemDelegate {
                clip: false
                height: parent.height * 2
                width: parent.width
                property string adm : model.admin
                property string membersRaw : model.members
                ListView {
                    leftMargin: 0
                    width: parent.width
                    height: parent.height / 2
                    displayMarginBeginning: 0
                    displayMarginEnd: 0
                    Layout.margins: 0
                    clip: false
                    verticalLayoutDirection: ListView.TopToBottom
                    model: membersRaw.split(",")
                    delegate: ItemDelegate {
                        width: parent.width
                        Label {
                           anchors.left: parent.left
                           id: nickname
                           text: modelData
                           font.pixelSize: 24
                           anchors.bottom: parent.bottom
                           color: "steelblue"
                           MouseArea {
                               anchors.fill: parent
                               onClicked: print("nameClicked")
                           }
                        }
                        Label {
                            anchors.bottom: parent.bottom
                            anchors.left: parent.left
                            anchors.leftMargin: 200
                            text: "admin"
                            visible: adm === modelData
                            color: "lightgrey"
                            font.pixelSize: 15
                        }

                        Text {
                            visible: adm === serverConnect.login() && serverConnect.login() !== modelData
                            anchors.rightMargin: 30
                            anchors.right: parent.right
                            id: removeLabel
                            text: "Remove"
                            font.pixelSize: 15
                            anchors.bottom: parent.bottom
                            MouseArea {
                                anchors.fill: parent
                                onClicked: editView.model.removeMember(modelData, contactId)
                            }
                       }
                        Rectangle { // bottom border
                                anchors.bottom: parent.bottom
                                height: 0.5
                                width: parent.width
                                color: "lightgrey"
                        }
                    }
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            //Layout.fillHeight: true
            TextArea {
                //anchors.top: parent.top
                Layout.fillWidth: true
                id: userIdField
                placeholderText: qsTr("Enter user ID")
                wrapMode: TextArea.Wrap
            }
            Button {
                //anchors.top: parent.top
                text: "add member"
                onClicked: editView.model.addMember(userIdField.text, contactId)
            }
        }
    }
}
