#include "logindialog.h"
#include <QApplication>
#include<QMessageBox>
#include"RemoteDatabaseHandler.h"
LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
}

void LoginDialog::setupUI()
{
    setWindowTitle("User Login");
    accountLabel = new QLabel("Username:");
    accountEdit = new QLineEdit();
    passwordLabel = new QLabel("Password:");
    passwordEdit = new QLineEdit();
    passwordEdit->setEchoMode(QLineEdit::Password);
    loginButton = new QPushButton("Login");

    QHBoxLayout *inputLayout = new QHBoxLayout;
    inputLayout->addWidget(accountLabel);
    inputLayout->addWidget(accountEdit);
    inputLayout->addWidget(passwordLabel);
    inputLayout->addWidget(passwordEdit);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(loginButton);

    setLayout(mainLayout);
    isServerOnline();
    connect(loginButton, &QPushButton::clicked, 
            this, &LoginDialog::on_loginButton_clicked);

}

void LoginDialog::on_loginButton_clicked()
{
    RemoteDatabaseHandler *temp = new RemoteDatabaseHandler("n2n.fornoone.xyz",5432);
    bool isTrue = temp->validateUser(accountEdit->text(),passwordEdit->text());
    if (isTrue) {
        accept();
    } else {
        // 连接失败，弹窗提示
        QMessageBox::warning(this, "Login Failed",
                             "Invalid username or password");
    }
    delete temp;
}
void LoginDialog::isServerOnline(){
    RemoteDatabaseHandler *temp = new RemoteDatabaseHandler("n2n.fornoone.xyz",5432);
    bool isOnline = temp->tryConnect();
    if (isOnline) {
        // 连接成功，弹窗提示
        QMessageBox::information(this, "连接成功", "服务器连接成功！");
    } else {
        // 连接失败，弹窗提示
        QMessageBox::warning(this, "连接失败", "服务器连接失败，请检查您的设置！");
        QCoreApplication::exit(1); // 退出程序，返回非零状态码
    }
    delete temp;
}
