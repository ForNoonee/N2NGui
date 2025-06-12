#ifndef REMOTE_DATABASE_HANDLER_H
#define REMOTE_DATABASE_HANDLER_H

#include <string>
#include <vector>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QCryptographicHash> // 添加头文件

class RemoteDatabaseHandler {
public:
    RemoteDatabaseHandler(const std::string& host, int port);
    ~RemoteDatabaseHandler();
    bool tryConnect();
    bool validateUser(const QString& username, const QString& password);
    QStringList getServerInfo();
    bool isConnected() const;
    bool reconnect();
    static QString computeHash(const QString& password);
private:
    QString host;
    int port;
    QSqlDatabase db;
     QString connectionName;
    struct DatabaseConfig {
        QString host;
        int port;
        QString dbName;
        QString user;
        QString password;
    };
     bool ensureConnection();
    DatabaseConfig loadDatabaseConfig();
};

#endif // REMOTE_DATABASE_HANDLER_H
