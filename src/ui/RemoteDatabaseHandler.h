#ifndef REMOTE_DATABASE_HANDLER_H
#define REMOTE_DATABASE_HANDLER_H

#include <string>
#include <vector>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

class RemoteDatabaseHandler {
public:
    RemoteDatabaseHandler(const std::string& host, int port);
    bool tryConnect();
    bool validateUser(const QString& username, const QString& password);
    QStringList getServerInfo();

private:
    std::string host;
    int port;
    QSqlDatabase db; // 使用Qt SQL数据库对象
};

#endif // REMOTE_DATABASE_HANDLER_H
