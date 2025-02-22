#include "ui/logindialog.h"
#include "ui/controlpanel.h"
#include <QApplication>
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    int argc = 0;
    char **argv = nullptr;

    QApplication app(argc, argv);

    // 显示登录对话框
    LoginDialog loginDialog;
    if (loginDialog.exec() == QDialog::Accepted) {
        ControlPanel controlPanel;
        controlPanel.show();
        return app.exec();
    }
    return 0;
}
