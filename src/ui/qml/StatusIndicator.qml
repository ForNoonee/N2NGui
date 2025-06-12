// StatusIndicator.qml
RowLayout {
    spacing: 15

    Label {
        text: `Status: ${root.connectionStatus}`
        color: {
            if (root.connectionStatus.includes("Connected")) return "green";
            if (root.connectionStatus.includes("Error")) return "orange";
            return "red";
        }
        font.bold: true
    }

    Label {
        text: `Latency: ${root.currentLatency}`
        font.italic: true
    }
}
