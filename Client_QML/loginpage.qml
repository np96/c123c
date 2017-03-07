import QtQuick 2.7
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.0
Page {
    ColumnLayout {
        id: loginPage
        anchors.centerIn: parent
        state: "LOGIN"

        Label {
            Layout.fillWidth: true
            id: errorMessage
            visible: false
        }

        TextField {
            Layout.fillWidth: true
            id: loginForm
            placeholderText: qsTr("login")
            validator: RegExpValidator {regExp: /^\w{4,12}$/}
        }

        TextField {
            Layout.fillWidth: true
            id: passwordForm
            placeholderText: qsTr("password")
            echoMode: TextInput.Password
        }

        RowLayout {
            id :buttons
            state: "LOGIN"
            Button {
                id: backButton
                visible: true
                text: "Sign up"
                onClicked: {
                    if (loginPage.state == "LOGIN") {
                        backButton.text = "Back"
                        loginPage.state = "SIGNUP"
                        authButton.text = "Sign up"
                    } else {
                        loginPage.state = "LOGIN"
                        backButton.text = "Sign up"
                        authButton.text = "Login"
                    }
                }
            }
            Button {
                Layout.fillWidth: true
                id: authButton
                text: "Login"
                onClicked: {
                    if (loginPage.state === "LOGIN") {
                        serverConnect.loginRequest(loginForm.text, passwordForm.text);
                    } else {
                        serverConnect.signUpRequest(loginForm.text, passwordForm.text);
                    }
                }
            }
        }
        Label {
            Layout.fillWidth: true
            Layout.fillHeight: true
            id: initialLabel
            font.pixelSize: 15
            anchors.verticalCenter: parent.verticalCenter
            text: qsTr("Choose group or create your own to start messaging")
        }
        states: [
            State {
                name: "LOGIN"
                PropertyChanges {target: initialLabel; visible: false}
                PropertyChanges {target: loginPage; visible: true}
                PropertyChanges {target: errorMessage; visible: false}
            },
            State {
                name: "SIGNUP"
                PropertyChanges {target: initialLabel; visible: false}
                PropertyChanges {target: loginPage; visible: true}
                PropertyChanges {target: errorMessage; visible: false}
            },
            State {
                name: "LOGGED IN"

                PropertyChanges {target: initialLabel; visible: true}
                PropertyChanges {target: loginForm; visible: false}
                PropertyChanges {target: passwordForm; visible: false}
                PropertyChanges {target: buttons; visible: false}
            }
        ]

        Connections {
            target: serverConnect
            onLoginSuccess: {
                if (ans === "") {
                    errorMessage.text = "Wrong login/password pair"
                    errorMessage.visible = true
                    errorMessage.color = "red"
                } else {
                    loginPage.state = "LOGGED IN"

                }
            }
            onSignupSuccess: {
                if (ans == false) {
                    errorMessage.text = "Couldn't sign up"
                    errorMessage.visible = true
                    errorMessage.color = "red"
                } else {
                    loginPage.state = "LOGIN"
                    errorMessage.text = "Now login"
                    errorMessage.visible = true
                    errorMessage.color = "blue"
                }

            }

        }
    }
}
