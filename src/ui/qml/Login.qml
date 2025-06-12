import QtQuick
import QtQuick.Controls 2.15
import QtQuick.Layouts 2.15
import Qt5Compat.GraphicalEffects
import QtQuick.Controls.Material 2.15

ApplicationWindow {
    id: root
    visible: true
    width: 360
    height: 640
    minimumWidth: 360
    minimumHeight: 560
    title: qsTr("ForNoone")
    color: "#F5F6FA"
    flags: Qt.Window | Qt.FramelessWindowHint

    // Material 主题配置
    Material.theme: Material.Light
    Material.accent: "#7E57C2"
    Material.elevation: 6

    // 配色方案
    readonly property color textColor: Material.foreground
    readonly property color hintColor: "#A0A4B8"

    Connections {
        target: backend

        function onLoginSuccess() {
            pageLoader.source = "ControlPanel.qml"
            root.visible = false
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

    Loader {
        id: pageLoader
        anchors.fill: parent
    }

    // 登录卡片容器
    Rectangle {
        id: loginContainer
        anchors.centerIn: parent
        width: Math.min(parent.width * 0.9, 400)
        height: contentLayout.height + 80
        radius: 24
        color: Material.backgroundColor
        layer.enabled: true
        layer.effect: DropShadow {
            radius: 24
            color: "#20000000"
            transparentBorder: true
        }

        ColumnLayout {
            id: contentLayout
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                margins: 32
            }
            spacing: 24

            // 标题部分
            ColumnLayout {
                spacing: 16
                Layout.alignment: Qt.AlignHCenter

                Image {
                    source: "qrc:///resource/icon.jpg"
                    Layout.preferredWidth: 72
                    Layout.preferredHeight: 72
                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    text: qsTr("欢迎回来")
                    //font: Material.font("headline6")
                    color: textColor
                }
            }

            // 输入表单
            ColumnLayout {
                spacing: 16
                Layout.fillWidth: true
                Layout.leftMargin: 15  // 统一左右边距
                Layout.rightMargin: 15

                // 用户名输入框
                CustomTextField {
                    id: accountField
                    placeholderText: qsTr("用户名或邮箱")
                    leftIcon: "qrc:///resource/user.svg"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 56
                    Layout.minimumWidth: 250  // 设置最小宽度
                }

                // 密码输入框
                CustomTextField {
                    id: passwordField
                    placeholderText: qsTr("密码")
                    leftIcon: "qrc:///resource/lock.svg"
                    echoMode: TextInput.Password
                    Layout.fillWidth: true
                    Layout.preferredHeight: 56
                    Layout.minimumWidth: 250  // 保证相同最小宽度
                }
            }

            // 协议和忘记密码
            RowLayout {
                Layout.fillWidth: true
                spacing: 16  // 控制输入框之间的垂直间距
                Layout.leftMargin: 16  // 左右边距统一
                Layout.rightMargin: 16

                CheckBox {
                    id: agreementCheck
                    text: qsTr("同意协议")
                    Material.foreground: hintColor
                    Material.accent: root.Material.accent
                }

                Item { Layout.fillWidth: true }

                TextButton {
                    text: qsTr("忘记密码?")
                    onClicked: {/* 忘记密码逻辑 */}
                }
            }

            // 登录按钮
            Button {
                id: loginButton
                text: qsTr("登录")
                Layout.fillWidth: true
                Layout.preferredHeight: 48
                enabled: agreementCheck.checked
                Material.background: Material.accent
                Material.foreground: "white"
                Material.elevation: 2

                onClicked: backend.validateLogin(accountField.text, passwordField.text)
            }
        }
    }

    // 底部注册提示
    Row {
        anchors {
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
            margins: 24
        }
        spacing: 4

        Text {
            text: qsTr("没有账号?")
            color: hintColor
            font.pixelSize: 14
        }

        TextButton {
            text: qsTr("立即注册")
            onClicked: {/* 跳转注册 */}
        }
    }

    // 自定义输入框组件
    component CustomTextField: TextField {
        property alias leftIcon: icon.source
        Material.background: "transparent"
        Material.accent: root.Material.accent
        // 统一尺寸设置
        leftPadding: 48
        rightPadding: 16
        font.pixelSize: 16
        color: root.textColor
        verticalAlignment: TextInput.AlignVCenter

        background: Rectangle {
            radius: 8
            color: Material.hintTextColor
            opacity: 0.1
            border.width: parent.activeFocus ? 2 : 1
            border.color: parent.activeFocus ? Material.accent : "transparent"
        }

        Image {
            id: icon
            anchors {
                left: parent.left
                leftMargin: 16
                verticalCenter: parent.verticalCenter
            }
            sourceSize: Qt.size(24, 24)
            opacity: 0.6
        }
    }

    // 文本按钮组件
    component TextButton: Text {
        property alias mouseArea: ma
        signal clicked

        font.pixelSize: 12
        color: hintColor

        MouseArea {
            id: ma
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: parent.clicked()
        }

        states: State {
            name: "hovered"
            when: ma.containsMouse
            PropertyChanges { target: parent; color: Material.accent }
        }
    }

    // 错误提示对话框
    Dialog {
        id: errorDialog
        title: qsTr("登录失败")
        standardButtons: Dialog.Ok
        Material.accent: Material.Red
    }

    Dialog {
        id: statusDialog
        title: qsTr("服务器状态")
        standardButtons: Dialog.Ok
    }
}
