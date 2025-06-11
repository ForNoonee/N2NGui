#include <windows.h>
#include <QApplication>
#include <QQmlApplicationEngine>
#include "ui/loginwidget.h"
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
    QApplication app(argc, argv); // 必须使用QApplication同时支持Widgets和QML
    // 创建QML引擎（必须使用动态分配保持生命周期）
    // qmlRegisterType<ControlPanelBackend>("Backend", 1, 0, "ControlPanelBackend");
    // QQmlApplicationEngine *engine = new QQmlApplicationEngine();

    // // 加载QML界面
    // engine->load(QUrl(QStringLiteral("qrc:/ControlPanel.qml")));
    // 注册QML类型（必须在QML引擎创建前完成）
 //    qmlRegisterType<ControlPanelBackend>("Backend", 1, 0, "ControlPanelBackend");
 //    qmlRegisterType<ServerInfoModel>("NetworkModels", 1, 0, "ServerInfoModel");
 //    // 创建登录窗口
 //    LoginWidget loginWindow;

 //    QObject::connect(&loginWindow, &LoginWidget::loginSuccessful, [&]() {
 //        // 关闭登录窗口
 //        loginWindow.close();

 //        // 创建QML引擎（必须使用动态分配保持生命周期）
 //        QQmlApplicationEngine *engine = new QQmlApplicationEngine();
 //        ControlPanelBackend *backend = new ControlPanelBackend();
 //        // 加载QML界面
 //        engine->rootContext()->setContextProperty("backend", backend);
 //        engine->load(QUrl(QStringLiteral("qrc:/ControlPanel.qml")));


 //        // 绑定窗口关闭事件
 //        QObject::connect(engine->rootObjects().first(), &QObject::destroyed,
 //                         engine, &QQmlApplicationEngine::deleteLater);

 //    });

 //    loginWindow.show();
 //    return app.exec();
       //qmlRegisterType<LoginBackend>("Backend", 1, 0, "loginBackend");
        qmlRegisterType<ControlPanelBackend>("Backend", 1, 0, "ControlPanelBackend");
        qmlRegisterType<ServerInfoModel>("NetworkModels", 1, 0, "ServerInfoModel");
        QQmlApplicationEngine *engine = new QQmlApplicationEngine();
        LoginBackend *backend = new LoginBackend();
        engine->rootContext()->setContextProperty("backend", backend);
        engine->load(QUrl(QStringLiteral("qrc:/Login.qml")));
       return app.exec();
}
