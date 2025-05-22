#include "RemoteDatabaseHandler.h"
#include <iostream>
#include <QProcess>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QVariant>
#include <QRegularExpression>
#include<QApplication>
#include<QSettings>
#include<QFileInfo>
#include<QDebug>
#include<QTcpSocket>
#include<QThread>
RemoteDatabaseHandler::RemoteDatabaseHandler(const std::string& host1, int port1)
    : connectionName(QString("PostgreSQL_%1").arg(quintptr(this)))
{
    // 加载配置文件
    DatabaseConfig config = loadDatabaseConfig();
    host=config.host;
    // 配置数据库连接
    if(!QSqlDatabase::contains(connectionName)) {
        db = QSqlDatabase::addDatabase("QPSQL", connectionName);
    } else {
        db = QSqlDatabase::database(connectionName);
    }

    db.setHostName(config.host);
    db.setPort(config.port);
    db.setDatabaseName(config.dbName);
    db.setUserName(config.user);
    db.setPassword(config.password);
}
bool RemoteDatabaseHandler::ensureConnection() {
    if(db.isOpen()) {
        QSqlQuery pingQuery("SELECT 1 AS connection_test", db);
        if(pingQuery.exec() && pingQuery.next()) {
            return true;
        } else {
            qCritical() << "check isConnected failed:"
                        << pingQuery.lastError().text();
        }
        qDebug() << "Connection stale, reconnecting...";
        db.close();
    }

    for(int i=0; i<3; ++i) {
        qDebug() << "try";
        if(db.open()) {
            if(db.isValid()) {
                qInfo() << "connected";
                return true;
            }
        }
        qCritical() << "连接失败:"
                    << db.lastError().driverText()
                    << "\n详细错误:"
                    << db.lastError().databaseText();
        QThread::msleep(1000);
    }
    return false;
}

RemoteDatabaseHandler::DatabaseConfig RemoteDatabaseHandler::loadDatabaseConfig()
{
    DatabaseConfig config;

    // 定义默认值
    const QString DEFAULT_HOST = "n2n.fornoone.xyz";
    const int DEFAULT_PORT = 5432;
    const QString DEFAULT_DBNAME = "n2n";
    const QString DEFAULT_USER = "postgres";
    const QString DEFAULT_PASSWORD = "Shuowoaini0123";

    // 查找配置文件路径
    QString configPath = QCoreApplication::applicationDirPath() + "/config.ini";

    if(!QFileInfo::exists(configPath)){
        qFatal("配置文件 config.ini 未找到！路径：%s", qUtf8Printable(configPath));
    }

    QSettings settings(configPath, QSettings::IniFormat);
    settings.beginGroup("Database");

    // 读取配置项（带空值检查）
    config.host = settings.value("Host").toString();
    if (config.host.trimmed().isEmpty()) {
        config.host = DEFAULT_HOST;
        qWarning() << "Host 未配置，使用默认值:" << DEFAULT_HOST;
    }

    bool portOk = false;
    config.port = settings.value("Port").toInt(&portOk);
    if (!portOk || config.port <= 0) {
        config.port = DEFAULT_PORT;
        qWarning() << "Port 配置无效，使用默认值:" << DEFAULT_PORT;
    }

    config.dbName = settings.value("DatabaseName").toString();
    if (config.dbName.trimmed().isEmpty()) {
        config.dbName = DEFAULT_DBNAME;
        qWarning() << "DatabaseName 未配置，使用默认值:" << DEFAULT_DBNAME;
    }

    config.user = settings.value("UserName").toString();
    if (config.user.trimmed().isEmpty()) {
        config.user = DEFAULT_USER;
        qWarning() << "UserName 未配置，使用默认值:" << DEFAULT_USER;
    }

    config.password = settings.value("Password").toString();
    if (config.password.trimmed().isEmpty()) {
        config.password = DEFAULT_PASSWORD;
        qWarning() << "Password not found";
    }

    settings.endGroup();
    return config;
}
bool RemoteDatabaseHandler::tryConnect() {
    // 使用QTcpSocket检测端口
    QTcpSocket socket;
    socket.connectToHost(host, port);
    return socket.waitForConnected(3000); // 3秒超时
}

bool RemoteDatabaseHandler::validateUser(const QString& username, const QString& password) {
    if(!ensureConnection()) {
        qCritical() << "数据库连接不可用";
        return false;
    }
     QSqlQuery query(db);
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
    if(!ensureConnection()) {
        qWarning() << "database disconnected";
        return serverInfo;
    }

    QSqlQuery query(db);
    if(!query.prepare("SELECT node_name, ip_address FROM servers")) {
        qCritical() << "sql bad:" << query.lastError().text();
        return serverInfo;
    }

    if(!query.exec()) {
        qCritical() << "failed:" << query.lastError().text()
            << "\nSQL:" << query.lastQuery();
        return serverInfo;
    }

    while(query.next()) {
        QString node = query.value("node_name").toString();
        QString ip = query.value("ip_address").toString();
        if(node.isEmpty() || ip.isEmpty()) {
            qWarning() << "null，skip";
            continue;
        }
        serverInfo.append(QString("%1 (%2)").arg(node).arg(ip));
    }

    qDebug() << "find" << serverInfo.size() << "serverlist";
    return serverInfo;
}
RemoteDatabaseHandler::~RemoteDatabaseHandler() {
    QSqlDatabase::database(connectionName).close();
    QSqlDatabase::removeDatabase(connectionName);
}
