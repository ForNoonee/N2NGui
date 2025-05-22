
#include "ui/controlpanel.h"
#include <QApplication>
#include <windows.h>
#include"ui/loginwidget.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    int argc = 0;
    char **argv = nullptr;

    QApplication app(argc, argv);


    // LoginDialog loginDialog;
    // if (loginDialog.exec() == QDialog::Accepted) {
    //     ControlPanel controlPanel;
    //     controlPanel.show();
    //     return app.exec();
    // }
    // return 0;
    // ControlPanel controlPanel;
    // controlPanel.show();
    //return app.exec();
    LoginWidget login;
  //  login.setStyleSheet("background-color: #291935;");
    login.show();
    ControlPanel controlPanel;
    QObject::connect(&login, &LoginWidget::loginSuccessful, &controlPanel, &ControlPanel::show);
    QObject::connect(&login, &LoginWidget::loginSuccessful, &controlPanel, &ControlPanel::updateServerList);
    return app.exec();
}
