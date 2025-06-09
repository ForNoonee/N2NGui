// ControlButtons.qml
RowLayout {
    spacing: 10

    Button {
        text: "Install Driver"
        onClicked: backend.installDriver()
    }

    Button {
        text: "Connect VPN"
        enabled: !backend.isConnected
        onClicked: backend.connectToVpn(root.selectedIP)
    }

    Button {
        text: "Disconnect"
        enabled: backend.isConnected
        onClicked: backend.disconnectVpn()
    }

    Button {
        text: "View Logs"
        onClicked: logWindow.open()
    }
}
