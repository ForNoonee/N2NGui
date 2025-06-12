// #include "loginwidget.h"
// #include "ui_loginwidget.h"
// #include<QMessageBox>
// #include"RemoteDatabaseHandler.h"
// #include <QApplication>
// #include<QFormLayout>


// #include"controlpanelwithoutuserserver.h"
// LoginWidget::LoginWidget(QWidget *parent)
//     : QWidget(parent)
//     , ui(new Ui::LoginWidget)
// {
//     ui->setupUi(this);
//     setstylesheet();
//     connect(ui->btn_login, &QPushButton::clicked, this, &LoginWidget::onLoginButtonClicked);
// }
// void LoginWidget::setstylesheet(){
//     ui->passwordLineEdit->setStyleSheet("background-color: #383359");
//     ui->accountLinedit->setStyleSheet("background-color: #483642");
//     ui->btn_login->setEnabled(false);
//     ui->accountLinedit->setStyleSheet(
//         "QLineEdit {"
//         "border: 2px solid #CCCCCC;"   // 边框颜色和宽度
//         "border-radius: 15px;"         // 圆角半径（像素值）
//         "padding: 5px 10px;"           // 内边距（上下 5px，左右 10px）
//         "background-color: #FFFFFF;"   // 背景色
//         "color: #333333;"              // 文字颜色
//         "}"
//         "QLineEdit:focus {"                // 输入框聚焦时的样式
//         "border: 2px solid #0066CC;"   // 聚焦时边框颜色
//         "}"
//         );
//     ui->passwordLineEdit->setStyleSheet(
//         "QLineEdit {"
//         "border: 2px solid #CCCCCC;"   // 边框颜色和宽度
//         "border-radius: 15px;"         // 圆角半径（像素值）
//         "padding: 5px 10px;"           // 内边距（上下 5px，左右 10px）
//         "background-color: #FFFFFF;"   // 背景色
//         "color: #333333;"              // 文字颜色
//         "}"
//         "QLineEdit:focus {"                // 输入框聚焦时的样式
//         "border: 2px solid #0066CC;"   // 聚焦时边框颜色
//         "}"
//         );
//     ui->btn_login->setStyleSheet(
//         "QPushButton {"
//         "background-color: white;"     // 默认背景色
//         "border: 1px solid #CCCCCC;"   // 边框颜色
//         "border-radius: 5px;"          // 圆角半径
//         "padding: 8px;"                // 内边距
//         "color: #333333;"              // 文字颜色
//         "}"
//         "QPushButton:hover {"              // 鼠标悬停时的样式
//         "background-color: #F5F5F5;"   // 悬停时的灰色背景
//         "}"
//         "QPushButton:pressed {"            // 按钮按下时的样式（可选）
//         "background-color: #E0E0E0;"   // 更深的灰色
//         "}"
//         );
//     QString imagePath = QCoreApplication::applicationDirPath() + "/icon.jpg";
//     QPixmap pix(imagePath);
//     if(pix.isNull()) {
//         pix = QPixmap(":/images/default.png");
//     }
//     ui->label->setPixmap(pix);
//     ui->btn_setting->hide();
// }
// LoginWidget::~LoginWidget()
// {
//     delete ui;
// }
// void LoginWidget::isServerOnline(){
//     RemoteDatabaseHandler *temp = new RemoteDatabaseHandler(m_currentServer.toStdString(),m_currentServerPort);
//     bool isOnline = temp->tryConnect();
//     if (isOnline) {
//         // 连接成功，弹窗提示
//         QMessageBox::information(this, "连接成功", "服务器连接成功！");
//     } else {
//         // 连接失败，弹窗提示
//         QMessageBox::warning(this, "连接失败", "服务器连接失败，请检查您的设置！");
//         //QCoreApplication::exit(1); // 退出程序，返回非零状态码
//     }
//     delete temp;
// }
// void LoginWidget::onLoginButtonClicked()
// {
//     RemoteDatabaseHandler *temp = new RemoteDatabaseHandler(m_currentServer.toStdString(),m_currentServerPort);
//     bool isValid = temp->validateUser(
//         ui->accountLinedit->text(),   // 通过ui访问
//         ui->passwordLineEdit->text()
//         );

//     if (isValid) {
//         emit loginSuccessful(); // 发出成功信号
//         this->close();          // 关闭登录窗口
//     } else {
//         QMessageBox::warning(this, "连接失败", "检查账户密码是否正确！");
//     }
//     delete temp;
// }




// void LoginWidget::on_checkBox_stateChanged(int arg1)
// {
//     if(ui->checkBox->isChecked()){
//         ui->btn_login->setEnabled(true);
//     }
//     else{
//         ui->btn_login->setEnabled(false);
//     }
// }


// void LoginWidget::on_btn_setting_clicked()
// {
//     // // 创建配置窗口
//     // QWidget *configWidget = new QWidget();
//     // configWidget->setWindowTitle("Server Setting");
//     // configWidget->setMinimumSize(300, 150);

//     // // 使用表单布局
//     // QFormLayout *formLayout = new QFormLayout(configWidget);

//     // // IP输入框
//     // QLineEdit *ipEdit = new QLineEdit(configWidget);
//     // ipEdit->setPlaceholderText("请输入IP地址");
//     // ipEdit->setText("192.168.100.1");  // 默认IP
//     // formLayout->addRow("用户数据IP:", ipEdit);

//     // // 端口输入框
//     // QLineEdit *portEdit = new QLineEdit(configWidget);
//     // portEdit->setPlaceholderText("请输入端口号");
//     // portEdit->setText("5432");  // 默认端口
//     // formLayout->addRow("端口:", portEdit);

//     // // 确认按钮
//     // QPushButton *confirmBtn = new QPushButton("确认", configWidget);
//     // formLayout->addRow(confirmBtn);

//     // // 连接确认按钮
//     // QObject::connect(confirmBtn, &QPushButton::clicked, [=](){
//     //     QString ip = ipEdit->text();
//     //     quint16 port = portEdit->text().toUShort();  // 转换为无符号短整型

//     //     // 发出带参数的信号
//     //     m_currentServer=ip;
//     //     m_currentServerPort=port;
//     //     isServerOnline();
//     //     configWidget->close();
//     //     configWidget->deleteLater();
//     // });

//     // configWidget->show();
// }


// void LoginWidget::on_pushButton_clicked()
// {
//     ControlPanelWithoutUserServer *panel = new ControlPanelWithoutUserServer();
//     connect(panel, &QObject::destroyed, this, &LoginWidget::show);
//     // 设置关闭时自动删除
//     panel->setAttribute(Qt::WA_DeleteOnClose);
//     // 显示为独立窗口
//     panel->show();
//     this->hide();
// }

