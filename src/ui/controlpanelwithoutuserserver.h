// // controlpanelwithoutuserserver.h
// #pragma once
// #include <QMainWindow>
// #include <QProcess>
// #include<QPushButton>
// #include<QLineEdit>
// #include<QLabel>
// class ControlPanelWithoutUserServer : public QMainWindow
// {
//     Q_OBJECT
// public:
//     explicit ControlPanelWithoutUserServer(QWidget *parent = nullptr);
//     ~ControlPanelWithoutUserServer();

// private slots:
//     void installDriver();
//     void connectToVpn();
//     void onDisconnectClicked();
//     void checkN2NStatus();

// private:
//     void setupUI();
//     void cleanupNetworkInterfaces();
//     bool hasN2NInterface();
//     bool isN2NConnected();
//     QString getN2NInterfaceIP();
//     bool validateIPFormat(const QString &ip);

//     // UI 组件
//     QPushButton *driverButton;
//     QPushButton *connectButton;
//     QPushButton *m_disconnectButton;
//     QLineEdit *serverInput;    // 服务器地址输入框（IP:PORT）
//     QLineEdit *localIpInput;   // 新增本地IP输入框
//     QLabel *m_statusLabel;
//     QTimer *m_statusTimer;
//     QProcess m_edgeProcess;
//     QString m_currentLocalIp;  // 当前使用的本地IP
// };
