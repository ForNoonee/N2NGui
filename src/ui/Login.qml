// Login.qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import Backend2 1.0  // 导入注册的模块

ApplicationWindow {
    id: root
    visible: true
    width: 400
    height: 600
    title: qsTr("登录")

    required property LoginBackend backend
    required property var bridge
    signal showControlPanel()

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 20

        Image {
            source: "qrc:/images/logo.png"
            Layout.preferredWidth: 120
            Layout.preferredHeight: 120
            fillMode: Image.PreserveAspectFit
        }

        TextField {
            id: accountField
            placeholderText: qsTr("账号")
            Layout.preferredWidth: 280
            Layout.preferredHeight: 50
            background: Rectangle {
                radius: 15
                border.color: accountField.activeFocus ? "#0066CC" : "#CCCCCC"
                border.width: 2
            }
            onTextChanged: backend.updateInputValidity()
        }

        TextField {
            id: passwordField
            placeholderText: qsTr("密码")
            echoMode: TextInput.Password
            Layout.preferredWidth: 280
            Layout.preferredHeight: 50
            background: Rectangle {
                radius: 15
                border.color: passwordField.activeFocus ? "#0066CC" : "#CCCCCC"
                border.width: 2
            }
            onTextChanged: backend.updateInputValidity()
        }

        CheckBox {
            id: agreementCheck
            text: qsTr("同意用户协议")
            onCheckedChanged: backend.setAgreementChecked(checked)
        }

        Button {
            id: loginButton
            text: qsTr("登录")
            enabled: backend.loginEnabled
            Layout.preferredWidth: 200
            Layout.preferredHeight: 40
            background: Rectangle {
                radius: 5
                color: loginButton.down ? "#E0E0E0" : "#FFFFFF"
                border.color: "#CCCCCC"
            }
            onClicked: {
                loginButton.enabled = false
                backend.validateLogin(accountField.text, passwordField.text)
            }
        }

        Button {
            text: qsTr("服务器设置")
            onClicked: serverDialog.open()
        }
    }

    Dialog {
        id: serverDialog
        title: qsTr("服务器配置")
        standardButtons: Dialog.Ok | Dialog.Cancel

        ColumnLayout {
            TextField {
                id: ipField
                text: backend.currentServer
                placeholderText: qsTr("服务器地址")
                validator: RegularExpressionValidator {
                    regularExpression: /^(\d{1,3}\.){3}\d{1,3}$/
                }
            }

            TextField {
                id: portField
                text: backend.currentServerPort
                placeholderText: qsTr("端口")
                validator: IntValidator { bottom: 1; top: 65535 }
            }
        }

        onAccepted: backend.updateServerConfig(ipField.text, parseInt(portField.text))
    }

    Connections {
        target: backend
        function onLoginSuccess() {
            root.showControlPanel()
            root.close()
        }

        function onLoginFailed(reason) {
            errorDialog.text = reason
            errorDialog.open()
            loginButton.enabled = true
        }

        function onServerOnlineChecked(online) {
            statusDialog.text = online ? "服务器在线" : "服务器离线"
            statusDialog.open()
        }
    }

    Dialog {
        id: errorDialog
        title: qsTr("登录失败")
        standardButtons: Dialog.Ok
    }

    Dialog {
        id: statusDialog
        title: qsTr("服务器状态")
        standardButtons: Dialog.Ok
    }
}
