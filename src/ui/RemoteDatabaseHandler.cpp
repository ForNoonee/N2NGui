#include "RemoteDatabaseHandler.h"
#include <iostream>
#include <QProcess>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QVariant>
#include <QRegularExpression>
RemoteDatabaseHandler::RemoteDatabaseHandler(const std::string& host, int port)
    : host(host), port(port) {
    db = QSqlDatabase::addDatabase("QPSQL"); // 使用PostgreSQL驱动
    db.setHostName(QString::fromStdString(host));
    db.setPort(port);
    db.setDatabaseName("n2n"); // 替换为实际数据库名
    db.setUserName("postgres"); // 替换为实际用户名
    db.setPassword("Shuowoaini0123"); // 替换为实际密码
}

bool RemoteDatabaseHandler::tryConnect() {
    // 使用ping命令验证服务器连接
    QProcess pingProcess;
    pingProcess.start("ping", QStringList() << QString::fromStdString(host));
    pingProcess.waitForFinished();
    QString output = pingProcess.readAllStandardOutput();
    return output.contains("TTL="); // 检查ping的输出
}

bool RemoteDatabaseHandler::validateUser(const QString& username, const QString& password) {
    if (!db.open()) {
        std::cerr << "Database connection failed: " << db.lastError().text().toStdString() << std::endl;
        return false;
    }

    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM users WHERE username = :username AND password = :password");
    query.bindValue(":username", username);
    query.bindValue(":password", password);
    query.exec();

    if (query.next()) {
        return query.value(0).toInt() > 0; // 返回是否存在匹配的用户
    }
    return false;
}

QStringList RemoteDatabaseHandler::getServerInfo() {
    QStringList serverInfo;
    if (!db.open()) {
        std::cerr << "Database connection failed: " << db.lastError().text().toStdString() << std::endl;
        return serverInfo;
    }

    QSqlQuery query("SELECT node_name, ip_address FROM servers"); // 选择需要的列
    while (query.next()) {
        QString info = "" + query.value(0).toString() +
                       ", " + query.value(1).toString(); // 读取两列
        serverInfo.append(info);  // 使用 QStringList 的 append 方法
    }

    db.close(); // 关闭数据库连接
    return serverInfo;
}
