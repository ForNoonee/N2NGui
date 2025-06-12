// loginbackend.cpp
#include "loginbackend.h"
#include <QtConcurrent/QtConcurrentRun>  // 在 loginbackend.cpp 头部添加

LoginBackend::LoginBackend(QObject* parent)
    : QObject(parent) {}

QString LoginBackend::currentServer() const {
    return m_currentServer;
}

quint16 LoginBackend::currentServerPort() const {
    return m_currentServerPort;
}

bool LoginBackend::loginEnabled() const {
    return m_loginEnabled;
}

void LoginBackend::setCurrentServer(QString server) {
    if (m_currentServer == server)
        return;
    m_currentServer = server;
    emit currentServerChanged(server);
}
bool LoginBackend::Onlogin(const QString& account, const QString& password){
    validateLogin(account,password);
    return 0;
}
void LoginBackend::setCurrentServerPort(quint16 port) {
    if (m_currentServerPort == port)
        return;
    m_currentServerPort = port;
    emit currentServerPortChanged(port);
}

void LoginBackend::validateLogin(const QString& account, const QString& password) {
    RemoteDatabaseHandler handler(
        m_currentServer.toStdString(),
        m_currentServerPort
        );

    bool isValid = handler.validateUser(account,password);

    if (isValid) {
        emit loginSuccess();
    } else {
        emit loginFailed("Invalid credentials");
    }
}

void LoginBackend::checkServerStatus() {
    QPointer<LoginBackend> guard(this); // 防止野指针

    QtConcurrent::run([guard]() {
        if(!guard) return;

        RemoteDatabaseHandler handler(
            guard->m_currentServer.toStdString(),
            guard->m_currentServerPort
            );

        bool online = handler.tryConnect();
        QMetaObject::invokeMethod(guard, [guard, online](){
            if(guard) guard->serverOnlineChecked(online);
        });
    });
}

void LoginBackend::updateServerConfig(const QString& ip, quint16 port) {
    setCurrentServer(ip);
    setCurrentServerPort(port);
    checkServerStatus();
}

void LoginBackend::setAgreementChecked(bool checked) {
    m_agreementChecked = checked;
    updateLoginEnableState();
}

void LoginBackend::updateLoginEnableState() {
    bool newState = m_agreementChecked && m_inputValid;
    if (newState != m_loginEnabled) {
        m_loginEnabled = newState;
        emit loginEnabledChanged(newState);
    }
}
