import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15 // 必须导入Material
import Backend 1.0 // 确保后端组件导入

Window {
    id: logWindow
    title: "运行日志"
    width: 600
    height: 400
    minimumWidth: 400
    minimumHeight: 300
    color: Material.BlueGrey
    //flags: Qt.Window | Qt.FramelessWindowHint
    // 确保窗口关闭时销毁实例
    //onClosing: destroy()

    ColumnLayout {
        anchors.fill: parent
        spacing: 5

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            TextArea {
                id: logContent
                text: backend.logs
                wrapMode: Text.Wrap
                readOnly: true
                font.family: "Consolas"
                selectByMouse: true

                // 自动滚动到底部
                onTextChanged: {
                    cursorPosition = text.length - 1
                    ensureVisible(cursorRectangle)
                }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: 10

            Button {
                text: "清空日志"
                onClicked: backend.clearLogs()
                Material.foreground: "white"
                Material.background: Material.Red
            }

            Button {
                text: "关闭"
                onClicked: logWindow.close()
            }
        }
    }
}
