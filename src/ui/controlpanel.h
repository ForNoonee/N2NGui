#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLabel>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QPushButton>
#include <QProcess>
#include <QLineEdit>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QFutureWatcher>

class ControlPanel : public QMainWindow {
    Q_OBJECT
public:
    explicit ControlPanel(QWidget *parent = nullptr);

private:
    QFutureWatcher<QStringList> *serverListWatcher;
    QTableWidget *serverTable;
    QLabel *latencyLabel;
    QLabel *speedLabel;
    QPushButton *driverButton;
    QPushButton *connectButton;
    QTimer *updateTimer;
    QNetworkAccessManager *networkManager;
    QProcess *edgeProcess;
    QLineEdit *ipAddressInput;
    void setupUI();
    void refreshServerList();
    void startConnectionTest();
    int pingServer(const QString &ipAddress);
    QFutureWatcher<int> *pingWatcher; // 用于监听 ping 结果
private slots:
      void updateServerList();
    void updateNetworkStatus();
   // void handleServerSelection(QTableWidgetItem *item);
    void installDriver();
     void connectToVpn();
    void handleServerListReady();
   //  void handleProcessOutput();
   //  void handleProcessError(QProcess::ProcessError error);
signals:
    void serverListReady(const QStringList &servers);
};

#endif // CONTROLPANEL_H
