#ifndef SERVERLISTUPDATER_H
#define SERVERLISTUPDATER_H

#include <QObject>
#include <QTimer>
#include <QStringList>

struct ServerInfo {
    QString name;
    QString ip;
    QString status; // 如 "Offline" 或 "XX ms"
};

class ServerListUpdater : public QObject
{
    Q_OBJECT
public:
    explicit ServerListUpdater(const QString &dbHost, int dbPort, QObject *parent = nullptr);
    ~ServerListUpdater();

public slots:
    void startUpdating();
    void stopUpdating();

signals:
    void serverListUpdated(const QList<ServerInfo> &servers);
    void errorOccurred(const QString &error);

private slots:
    void performUpdate();

private:
    int pingServer(const QString &hostName);

    QTimer *timer;
    QString dbHost;
    int dbPort;
};

#endif // SERVERLISTUPDATER_H
