import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0
LoginPage {
    Page {
        id: helloPage
        ColumnLayout {
            state: "LOGIN"
            TextField {
                Layout.fillWidth: true
                id: loginForm
                placeholderText: qStr("login")
                validator: RegExpValidator {regExp: /^\w{4,12}$/}
            }

            TextField {
                Layout.fillWidth: true
                id: passwordForm
                placeholderText: qStr("password")
                echoMode: TextInput.Password
            }

            Label {
                Layout.fillWidth: true
                Layout.fillHeight: true
                id: initialLabel
                font.pixelSize: 15
                anchors.verticalCenter: parent.verticalCenter
                text: "Choose group or create your own to start messaging"
            }

            RowLayout {
                Button {
                    Layout.fillHeight: true
                    id: backButton
                    visible: false
                }
                Button {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    id: authButton
                    text: "Login"
                    onClicked: {
                        if (authButton.state === "LOGIN") {
                        } else {

                        }
                    }
                }
           }
            states: [
                State {
                    name: "LOGIN"
                    PropertyChanges {target: backButton; visible: true}
                    PropertyChanges {target: authButton; text: "Login"; Layout.fillWidth: true; visible: true}
                    PropertyChanges {target: loginForm; visible: true}
                    PropertyChanges {target: passwordForm; visible: true}
                    PropertyChanges {target: initialLabel; visible: false}
                },
                State {
                    name: "SIGNUP"
                    PropertyChanges {target: backButton; visible: false}
                    PropertyChanges {target: authButton; text: "Sign up"; Layout.fillWidth: false; visible: true}
                    PropertyChanges {target: loginForm; visible: true}
                    PropertyChanges {target: passwordForm; visible: true}
                    PropertyChanges {target: initialLabel; visible: false}
                },
                State {
                    name: "LOGGED IN"
                    PropertyChanges {target: initialLabel; visible: true}
                    PropertyChanges {target: backButton; visible: false}
                    PropertyChanges {target: loginForm; visible: false}
                    PropertyChanges {target: passwordForm; visible: false}
                    PropertyChanges {target: authButton; visible: false}
                }

           ]
        }
    }
}
