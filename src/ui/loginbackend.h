// loginbackend.h
#include <QObject>
#include <QString>
#include <QDebug>
#include "RemoteDatabaseHandler.h"

class LoginBackend : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString currentServer READ currentServer WRITE setCurrentServer NOTIFY currentServerChanged)
    Q_PROPERTY(quint16 currentServerPort READ currentServerPort WRITE setCurrentServerPort NOTIFY currentServerPortChanged)
    Q_PROPERTY(bool loginEnabled READ loginEnabled NOTIFY loginEnabledChanged)

public:
    explicit LoginBackend(QObject* parent = nullptr);
    // 属性访问器
    QString currentServer() const;
    quint16 currentServerPort() const;
    bool loginEnabled() const;
    void setCurrentServer(QString server);
    void setCurrentServerPort(quint16 port);
public slots:
    Q_INVOKABLE bool Onlogin(const QString& account, const QString& password);
    void validateLogin(const QString& account, const QString& password);
    void checkServerStatus();
    void updateServerConfig(const QString& ip, quint16 port);
    void setAgreementChecked(bool checked);

signals:
    void loginSuccess();
    void loginFailed(const QString& reason);
    void serverOnlineChecked(bool isOnline);
    void currentServerChanged(QString currentServer);
    void currentServerPortChanged(quint16 currentServerPort);
    void loginEnabledChanged(bool loginEnabled);

private:
    void updateLoginEnableState();

    QString m_currentServer = "n2n.fornoone.xyz";
    quint16 m_currentServerPort = 5432;
    bool m_agreementChecked = false;
    bool m_inputValid = false;
    bool m_loginEnabled = false;
};
