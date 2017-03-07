import QtQuick 2.7
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.0
import cpp.chat.qml 1.0

ApplicationWindow {
    id: window
    visible: true

    width: 640


    height: 480

    title: qsTr("Hello World")

    property SqlGroupModel groupModel : SqlGroupModel {}
    property SqlContactModel contactModel : SqlContactModel {}

    GroupList {
        visible: false
        width: 105
        height: parent.height
        //Layout.fillWidth: true
        id: initialList
    }
    RowLayout {
        anchors.leftMargin: 105
        anchors.fill: parent
        StackView {
            Layout.fillHeight: true
            Layout.fillWidth: true
            visible: false
            id: stckView
            /*initialItem: GroupList {
              //  Layout.fillWidth: true
              //  Layout.fillHeight: true
                id: initialList     //todo.
                anchors.verticalCenter: parent.verticalCenter
            }*/
            initialItem: Label {
                font.pixelSize: 15
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                text: "Choose group or start your own"
                id: initialLabel
            }
        }
    }
    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: LoginPage {}

    }

    Connections {
        target: serverConnect
        onLoginSuccess: {
            print(ans)
            if (ans !== "") {
                stackView.visible = false
                stckView.visible = true
                initialList.visible = true
                serverConnect.getGroupsRequest()
                //groupsView.model.init()
            }
        }
    }

}
