import QtQuick 6
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import Backend 1.0
import NetworkModels 1.0

ApplicationWindow {
    visible: true
    width: 800
    height: 600
    title: "N2N组网控制面板"
    Material.theme: Material.Light
    Material.accent: Material.Blue
    id: mainwindow
    ControlPanelBackend {
        id: backend
        onConnectionStatusChanged: {
            statusText.text = backend.connectionStatus || "未连接"
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        // 状态显示区域
        RowLayout {
            Layout.fillWidth: true
            Layout.margins: 10
            spacing: 20

            Label {
                text: "状态:"
                font.bold: true
                Material.foreground: Material.Grey
            }

            Label {
                id: statusText
                text: backend.connectionStatus || "未连接"
                color: backend.connected ? "green" : "red"
            }

            Label {
                text: "当前IP:"
                font.bold: true
                Material.foreground: Material.Grey
            }

            Label {
                id: currentIP
                text: backend.currentIp ?  backend.currentIp : "未检测到有效 IP"

                BusyIndicator {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    running: backend.connectionStatus === "Connecting..." || (backend.currentIp === "" && backend.connectionStatus === "Connected")
                    width: 16
                    height: 16
                }
            }
        }

        // 本地IP输入区域（始终显示）
        TextField {
            id: localIpInput
            Layout.fillWidth: true
            placeholderText: "输入本地虚拟IP（例如：192.166.x.x），组网用户第一个x的值需要相同"
            text:"192.166.100.2"
            // 增强版IPv4正则表达式
            validator: RegularExpressionValidator {
                regularExpression: /^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/
            }

            // 实时验证指示
            Material.background: {
                if (text.length === 0) return Material.LightGray
                return acceptableInput ? Material.LightGreen : Material.Pink
            }

            // IP变化时触发验证
            onTextChanged: {
                console.log("IP变化:", text, "有效:", acceptableInput)
                Qt.callLater(connectBtn.validateState) // 延迟确保状态更新
            }
        }

        // 自定义服务器输入区域
        GroupBox {
            Layout.fillWidth: true
            Layout.margins: 10
            title: "自定义服务器"
            visible: customAddressInput.visible

            TextField {
                id: customAddressInput
                width: parent.width
                placeholderText: "服务器地址（IP:端口）"
                validator: RegularExpressionValidator {
                    regularExpression: /^(?:\d{1,3}\.){3}\d{1,3}:\d{1,5}$/
                }
                Material.foreground: Material.Orange
            }
        }

        // 服务器列表
        ListView {
            id: serverList
            currentIndex: -1
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: backend.serverModel
            highlight: Rectangle {
                color: Material.accent
                opacity: 0.3
                radius: 5
            }

            delegate: ItemDelegate {
                width: parent.width
                height: 60
                highlighted: ListView.isCurrentItem

                Row {
                    spacing: 30
                    anchors.verticalCenter: parent.verticalCenter
                    leftPadding: 20

                    Label {
                        text: model.name || "未知服务器"
                        width: 150
                        font.bold: true
                        Material.foreground: Material.Grey
                    }

                    // Label {
                    //     text: model.ip || "0.0.0.0:0"
                    //     width: 200
                    // }

                    Label {
                        text: model.latency > 0 ? model.latency + " ms" : "测试中..."
                        width: 120
                        color: model.latency < 100 ? "green" :
                               model.latency < 200 ? "orange" : "red"
                    }

                    Label {
                        text: model.status || "离线"
                        color: model.status === "在线" ? "green" : "red"
                    }
                }

                onClicked: {
                    ListView.view.currentIndex = index
                    customAddressInput.text = "" // 选择列表时清空自定义输入
                    console.log("点击索引:", index)
                }
            }
        }
        //日志
        Dialog {
            id: logDialog
            title: "运行日志"
            width: parent.width * 0.9
            height: parent.height * 0.7
            modal: true

            ScrollView {
                anchors.fill: parent
                TextArea {
                    id: logDisplay
                    text: backend.logs
                    wrapMode: Text.Wrap
                    readOnly: true
                    font.family: "Consolas"
                    selectByMouse: true
                }
            }

            standardButtons: Dialog.Close
        }

        // 操作按钮区域
        RowLayout {
            Layout.fillWidth: true
            Layout.margins: 10
            spacing: 15

            Button {
                text: "安装驱动"
                onClicked: backend.installDriver()
                Material.background: Material.Blue
                Material.foreground: "white"
            }

            Button {
                text: "显示日志"
                onClicked: {
                    let component = Qt.createComponent("LogView.qml")
                    if (component.status === Component.Ready) {
                        // 关键修改：设置父对象为主窗口
                        let win = component.createObject(mainwindow, {
                            "backend": backend
                        })
                        win.show()
                    }
                }
            }

            Button {
                id: connectBtn
                text: "连接服务器"

                // 修正后的启用条件
                enabled: {
                    const ipValid = localIpInput.acceptableInput
                    const hasValidServer = serverList.currentIndex >= 0
                    console.log("Enabled check:", !backend.connected, ipValid, hasValidServer, customAddressInput.acceptableInput )
                    return !backend.connected && ipValid && hasValidServer
                }

                // 增强的状态提示
                ToolTip {
                    visible: !parent.enabled
                    text: {
                        const reasons = []
                        if(backend.connected) reasons.push("已连接")
                        if(!localIpInput.acceptableInput) reasons.push("本地IP无效")
                        if(ListView.currentIndex < 0 && !customAddressInput.acceptableInput)
                            reasons.push("需要选择服务器或输入地址")
                        return "禁用原因: " + reasons.join("，")
                    }
                }

                // 优化点击处理
                onClicked: {
                    let serverIp = ""
                    let validationPass = true

                    // 优先使用列表选择
                    if (serverList.currentIndex >= 0) {
                        // 正确调用方式：仅传递行和列，省略父索引参数
                        const modelIndex = backend.serverModel.index(serverList.currentIndex, 0)


                            const serverName = backend.serverModel.data(modelIndex, ServerInfoModel.NameRole)
                            const serverIp = backend.serverModel.data(modelIndex, ServerInfoModel.IpRole)
                            console.log("服务器信息：", serverName, serverIp)
                    backend.connectToVpn(serverIp, localIpInput.text)
                    console.log("连接信息：", serverIp, localIpInput.text)
                    }
                }
            }

            Button {
                text: "断开连接"
                enabled: backend.connected
                onClicked: backend.disconnectVpn()
                Material.background: enabled ? Material.Red : Material.Grey
                Material.foreground: "white"
            }

            Button {
                text: "刷新列表"
                onClicked: backend.refreshServerList()
                Material.background: Material.Orange
                Material.foreground: "white"
            }
        }
    }
}
