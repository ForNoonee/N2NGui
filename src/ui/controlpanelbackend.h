// controlpanel_backend.h
#ifndef CONTROLPANEL_BACKEND_H
#define CONTROLPANEL_BACKEND_H

#include <QObject>
#include <QAbstractListModel>
#include <QProcess>
#include <QFutureWatcher>
#include <QTimer>
#include "RemoteDatabaseHandler.h"
#include "latencytester.h"

class ServerInfoModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        IpRole,
        LatencyRole,
        StatusRole
    };
    Q_ENUM(Roles)

    explicit ServerInfoModel(QObject *parent = nullptr) : QAbstractListModel(parent) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void updateServer(const QString &ip, int latency, const QString &status);
    void resetData(const QVector<QStringList> &servers);

private:
    QVector<QStringList> m_servers;
    QHash<QString, int> m_latencies;
    QHash<QString, QString> m_statuses;
};

class ControlPanelBackend : public QObject {
    Q_OBJECT
    Q_PROPERTY(ServerInfoModel* serverModel READ serverModel NOTIFY serverModelChanged)
    Q_PROPERTY(QString connectionStatus READ connectionStatus NOTIFY connectionStatusChanged)
    Q_PROPERTY(QString currentIp READ currentIp NOTIFY currentIpChanged)
    Q_PROPERTY(QString logs READ logs NOTIFY logsChanged)

public:
    explicit ControlPanelBackend(QObject *parent = nullptr);
    ~ControlPanelBackend();

    ServerInfoModel* serverModel() const { return m_serverModel; }
    QString connectionStatus() const { return m_connectionStatus; }
    QString currentIp() const { return m_currentIp; }
    QString logs() const { return m_logs; }

    Q_INVOKABLE void refreshServerList();
    Q_INVOKABLE void connectToVpn(const QString &serverIp, const QString &localIp);
    Q_INVOKABLE void disconnectVpn();
    Q_INVOKABLE void installDriver();

signals:
    void serverModelChanged();
    void connectionStatusChanged();
    void currentIpChanged();
    void logsChanged();
    void showErrorMessage(const QString &msg);
    void startLatencyTest(const QString &ip);

private slots:
    void handleServerListReady();
    void handleProcessOutput();
    void handleProcessError();
    void checkN2NStatus();
    void handleLatencyResult(const QString &ip, int latency);

private:
    void startConnectionTest();
    bool enableNetworkAdapter();
    bool hasN2NInterface();
    QString getN2NInterfaceIP();
    void cleanupNetworkInterfaces();
    void appendLog(const QString &log);
    ServerInfoModel *m_serverModel;
    RemoteDatabaseHandler m_dbHandler;
    QFutureWatcher<QStringList> *m_serverListWatcher;
    LatencyTester *m_latencyTester;
    QThread *m_testThread;
    QProcess m_edgeProcess;
    QTimer *m_statusTimer;
    QString m_logs;
    QString m_connectionStatus;
    QString m_currentIp;
    QString m_currentServerIp;
};

#endif // CONTROLPANEL_BACKEND_H
