import QtQuick 2.7
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.0
import cpp.chat.qml 1.0
Page {
    id: chatPage
    RowLayout {
        GroupList {
            id: groups
        }
        StackView {
            id: stckView
            anchors.left: groups.right
            anchors.leftMargin: 30
            initialItem: Label {
                Layout.fillWidth: true
                Layout.fillHeight: true
                id: initialLabel
                font.pixelSize: 15
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Choose group or create your own to start messaging")
            }
        }

    }

}
