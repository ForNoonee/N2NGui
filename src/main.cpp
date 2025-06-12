#include <windows.h>
#include<QGuiApplication>
#include <QQmlApplicationEngine>
#include "ui/controlpanelbackend.h"
#include <QQmlContext>
#include "ui/loginbackend.h"

// 必须导出WinMain的符号
#if defined(Q_CC_MSVC)
#pragma comment(linker, "/subsystem:windows")
#endif

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // 初始化Qt应用（必须手动构造参数）
    int argc = 0;
    char **argv = nullptr;
    QGuiApplication app(argc, argv);
        qmlRegisterModule("VirtualNetwork.GUI", 1, 0);
        qmlRegisterType<ControlPanelBackend>("Backend", 1, 0, "ControlPanelBackend");
        qmlRegisterType<ServerInfoModel>("NetworkModels", 1, 0, "ServerInfoModel");

        QQmlApplicationEngine *engine = new QQmlApplicationEngine();
        //engine->addImportPath("qrc:/");
        LoginBackend *backend = new LoginBackend();
        engine->rootContext()->setContextProperty("backend", backend);
        engine->load(QUrl(QStringLiteral("qrc:/qml/Login.qml")));
       return app.exec();
}
