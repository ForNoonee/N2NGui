// #ifndef LOGINWIDGET_H
// #define LOGINWIDGET_H

// #include <QWidget>

// namespace Ui {
// class LoginWidget;
// }

// class LoginWidget : public QWidget
// {
//     Q_OBJECT

// public:
//     explicit LoginWidget(QWidget *parent = nullptr);
//     ~LoginWidget();

// private slots:
//     void onLoginButtonClicked();
//     void on_checkBox_stateChanged(int arg1);

//     void on_btn_setting_clicked();

//     void on_pushButton_clicked();

// signals:
//     void loginSuccessful();
//    //  void serverConfigChanged(const QString& ip, quint16 port);
// private:
//     Ui::LoginWidget *ui;
//     void isServerOnline();
//    void setstylesheet();
//     QString m_currentServer = "n2n.fornoone.xyz";
//    quint16 m_currentServerPort = 5432;
//     struct DatabaseConfig {
//         QString host;
//         int port;
//         QString dbName;
//         QString user;
//         QString password;
//     };
// };

// #endif // LOGINWIDGET_H
