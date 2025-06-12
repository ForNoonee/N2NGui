// #ifndef CONTROLPANEL_H
// #define CONTROLPANEL_H

// #include <QMainWindow>
// #include <QTableWidget>
// #include <QLabel>
// #include <QTimer>
// #include <QNetworkAccessManager>
// #include <QPushButton>
// #include <QProcess>
// #include <QLineEdit>
// #include <QFuture>
// #include <QFutureWatcher>
// #include <QtConcurrent>
// #include <QFutureWatcher>
// #include <QPlainTextEdit>  // 新增日志组件
// #include <QDialog>         // 日志窗口
// #include "LatencyTester.h"

// class ControlPanel : public QMainWindow {
//     Q_OBJECT
// public:
//     explicit ControlPanel(QWidget *parent = nullptr);
//     ~ControlPanel();
// private:
//     QFutureWatcher<QStringList> *serverListWatcher;
//     QTableWidget *serverTable;
//     QLabel *latencyLabel;
//     QLabel *speedLabel;
//     QPushButton *driverButton;
//     QPushButton *connectButton;
//     QTimer *updateTimer;
//     QNetworkAccessManager *networkManager;
//     QProcess *edgeProcess;
//     QLineEdit *ipAddressInput;
//     void setupUI();
//     void refreshServerList();
//     void startConnectionTest();
// //    int pingServer(const QString &ipAddress);
//     QFutureWatcher<int> *pingWatcher; // 用于监听 ping 结果
//     bool hasN2NInterface();
//     QString getN2NInterfaceIP();
//     bool isN2NConnected();
//     QLabel *m_statusLabel;  // 连接状态标签
//     QTimer *m_statusTimer;  // 状态检测定时器
//     QProcess m_edgeProcess; // 替换 CreateProcess 为 QProcess
//     QPlainTextEdit *m_logViewer; // 日志显示组件
//     QDialog *m_logDialog;   // 日志窗口
//     QPushButton *m_disconnectButton; // 新增断开按钮
//     void cleanupNetworkInterfaces();
//     QFutureWatcher<int> *m_latencyWatcher;  // 新增延迟检测异步监控
//     QString m_currentServerIp;             // 记录当前连接服务器IP
//     QMap<QString, QTableWidgetItem*> m_serverStatusItems;  // IP到状态项的映射
//     LatencyTester* m_latencyTester;
//     QThread* m_testThread;
//     QString m_currentLocalIp;
//     bool isValidAddress(const QString& address) const; // 声明验证函数
// private slots:
//     void checkN2NStatus();  // 新增状态检测槽函数
//     void showLogWindow();   // 显示日志窗口
//     void handleDisconnectCompleted();
//     void updateNetworkStatus();
//     void handleLatencyResult(const QString &ip, int latency);
//    // void handleServerSelection(QTableWidgetItem *item);
//     void installDriver();
//      void connectToVpn();
//     void handleServerListReady();
//      void onDisconnectClicked();
//     void init();
//    //  void handleProcessOutput();
//    //  void handleProcessError(QProcess::ProcessError error);
// public Q_SLOTS:
//     void updateServerList();
// signals:
//     void serverListReady(const QStringList &servers);
//     void startLatencyTest(QString ipAddress);
// };

// #endif // CONTROLPANEL_H
