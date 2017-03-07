import QtQuick 2.7
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.0
import cpp.chat.qml 1.0
import QtQuick.Controls.Styles 1.4
/*Page {
    id: messagesPage
    property string group

}
*/

Page {
    Layout.fillHeight: true
    Layout.fillWidth: true

    property SqlGroupModel mdl
    id: root
    property string group
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
                stckView.pop(StackView.Immediate)
            }
        }
        Label {
            //anchors.verticalCenter: backButton.verticalCenter
            id: msgListTitle
            text: root.group
            font.pixelSize: 20
            anchors.centerIn: parent
        }
    }
    ColumnLayout {
        anchors.fill: parent
        ListView {
            anchors.leftMargin: 5
            Layout.fillWidth: true
            Layout.fillHeight: true
            displayMarginBeginning: 20
            displayMarginEnd: 20
            Layout.margins: 0//pane.leftPadding + messageField.leftPadding
            clip: true
            verticalLayoutDirection: ListView.TopToBottom
            id: msgsView
            spacing: 6

            Component.onCompleted: {
                console.log("This prints just fine!")
                msgsView.positionViewAtBeginning()
            }
            onCountChanged: {
                msgsView.currentIndex = count - 1
            }
            model: root.mdl
            delegate: Column {
                clip: true
                width: parent.width
                anchors.left: parent.left
                spacing: 3
                RowLayout {
                    width: parent.width
                    Label {
                        Layout.maximumWidth: implicitWidth
                        Layout.maximumHeight: implicitHeight
                        Layout.alignment: Qt.AlignLeft
                        leftPadding: 12
                        id: nameText
                        text: model.sentby
                        color: "steelblue"
                    }
                    Label {
                        Layout.alignment: Qt.AlignRight
                        rightPadding: 12
                        id: timestampText
                        text: Qt.formatDateTime(model.timestamp, "d MMM hh:mm")
                        color: "lightgrey"
                    }
                }
                Row {
                   id: messageRow
                   spacing: 3
                   anchors.right: undefined
                   Rectangle {
                       width: Math.min(messageText.implicitWidth + 24, msgsView.width - messageRow.spacing)
                       height: messageText.implicitHeight + 24
                       color: "white"
                       Label {
                           id: messageText
                           text: model.content
                           color: "black"
                           anchors.fill: parent
                           anchors.leftMargin: 12
                           anchors.rightMargin: 12
                           anchors.topMargin: 6
                           anchors.bottomMargin: 0
                           wrapMode: Label.Wrap
                       }
                   }
               }


           }

           ScrollBar.vertical: ScrollBar {}
        }
        Pane {
            id: pane
            Layout.fillWidth: true
            RowLayout {
               width: parent.width
               TextArea {
                   Layout.fillWidth: true
                   id: messageField
                   placeholderText: qsTr("Compose message")
                   wrapMode: TextArea.Wrap
               }
               Button {
                   id: sendButton
                   text: qsTr("Send")
                   enabled: messageField.text.trim().length > 0
                   onClicked: {
                        print(msgsView.model.group)
                        msgsView.model.sendMessage(group, messageField.text.trim());
                        messageField.text = "";
                  }
               }
            }
        }
    }
}
