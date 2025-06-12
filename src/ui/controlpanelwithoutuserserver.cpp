// // controlpanelwithoutuserserver.cpp
// #include "controlpanelwithoutuserserver.h"
// #include <QVBoxLayout>
// #include <QMessageBox>
// #include <QCoreApplication>
// #include <QRegularExpressionValidator>
// #include <winsock2.h>
// #include <Windows.h>
// #include <ws2tcpip.h>
// #include <iphlpapi.h>
// #include <IcmpAPI.h>
// #include <cfgmgr32.h>
// #include<QFormLayout>
// #include<QTimer>
// #include<QDebug>
// #pragma comment(lib, "Cfgmgr32.lib")

// ControlPanelWithoutUserServer::ControlPanelWithoutUserServer(QWidget *parent)
//     : QMainWindow(parent)
// {
//     setupUI();
// }

// ControlPanelWithoutUserServer::~ControlPanelWithoutUserServer()
// {
//     if (m_edgeProcess.state() == QProcess::Running) {
//         m_edgeProcess.kill();
//         m_edgeProcess.waitForFinished(1000);
//     }

//     // 清理时使用当前记录的本地IP
//     if(!m_currentLocalIp.isEmpty()){
//         QString command = QString("Remove-NetIPAddress -IPAddress %1 -ErrorAction SilentlyContinue")
//         .arg(m_currentLocalIp);
//         QProcess::execute("powershell", QStringList() << "-Command" << command);
//     }
// }

// void ControlPanelWithoutUserServer::setupUI()
// {
//     // 服务器地址输入
//     serverInput = new QLineEdit(this);
//     serverInput->setPlaceholderText("Supernode地址 (IP:端口)");
//     serverInput->setText("n2n.fornoone.xyz:5645");

//     // 本地IP输入
//     localIpInput = new QLineEdit(this);
//     localIpInput->setPlaceholderText("本地虚拟IP地址");
//     localIpInput->setText("192.168.100.1");

//     // IP格式验证器
//     QRegularExpressionValidator *ipValidator = new QRegularExpressionValidator(
//         QRegularExpression("^\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}$"),
//         localIpInput
//         );
//     localIpInput->setValidator(ipValidator);

//     // 按钮
//     driverButton = new QPushButton("安装驱动", this);
//     connectButton = new QPushButton("连接VPN", this);
//     m_disconnectButton = new QPushButton("断开连接", this);
//     m_disconnectButton->setEnabled(false);

//     // 状态显示
//     m_statusLabel = new QLabel("VPN Status: disconnect", this);
//     m_statusLabel->setStyleSheet("color: red; font-weight: bold;");

//     // 布局
//     QFormLayout *formLayout = new QFormLayout();
//     formLayout->addRow("服务器地址:", serverInput);
//     formLayout->addRow("本地IP地址:", localIpInput);

//     QHBoxLayout *buttonLayout = new QHBoxLayout();
//     buttonLayout->addWidget(driverButton);
//     buttonLayout->addWidget(connectButton);
//     buttonLayout->addWidget(m_disconnectButton);

//     QVBoxLayout *mainLayout = new QVBoxLayout();
//     mainLayout->addLayout(formLayout);
//     mainLayout->addLayout(buttonLayout);
//     mainLayout->addWidget(m_statusLabel);

//     QWidget *centralWidget = new QWidget();
//     centralWidget->setLayout(mainLayout);
//     setCentralWidget(centralWidget);

//     // 信号连接
//     connect(driverButton, &QPushButton::clicked, this, &ControlPanelWithoutUserServer::installDriver);
//     connect(connectButton, &QPushButton::clicked, this, &ControlPanelWithoutUserServer::connectToVpn);
//     connect(m_disconnectButton, &QPushButton::clicked, this, &ControlPanelWithoutUserServer::onDisconnectClicked);

//     // 状态检测定时器
//     m_statusTimer = new QTimer(this);
//     connect(m_statusTimer, &QTimer::timeout, this, &ControlPanelWithoutUserServer::checkN2NStatus);
//     m_statusTimer->start(5000);
// }

// void ControlPanelWithoutUserServer::connectToVpn()
// {
//     // 验证服务器地址格式
//     QString server = serverInput->text().trimmed();
//     QStringList serverParts = server.split(":");
//     if(serverParts.size() != 2 || !validateIPFormat(serverParts[0])){
//         QMessageBox::warning(this, "格式错误", "请输入有效的服务器地址\n示例：192.168.1.100:5645");
//         return;
//     }

//     // 验证本地IP格式
//     QString localIp = localIpInput->text().trimmed();
//     if(!validateIPFormat(localIp)){
//         QMessageBox::warning(this, "IP格式错误", "请输入有效的本地IP地址\n示例：192.168.100.50");
//         return;
//     }

//     // 停止已有进程
//     if(m_edgeProcess.state() == QProcess::Running){
//         m_edgeProcess.kill();
//     }

//     m_currentLocalIp = localIp; // 记录当前使用的本地IP

//     // 准备启动参数
//     QString edgePath = QCoreApplication::applicationDirPath() + "/edge.exe";
//     QStringList args = {
//         "-c", "mynetwork",
//         "-k", "mysecret",
//         "-a", localIp,
//         "-l", server
//     };

//     // 启动进程
//     m_edgeProcess.start(edgePath, args);

//     // 更新状态
//     m_statusLabel->setText("正在连接...");
//     m_statusLabel->setStyleSheet("color: blue;");
//     connectButton->setEnabled(false);
// }

// bool ControlPanelWithoutUserServer::validateIPFormat(const QString &ip)
// {
//     QRegularExpression regex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}"
//                              "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
//     return regex.match(ip).hasMatch();
// }
// bool ControlPanelWithoutUserServer::hasN2NInterface() {
//     ULONG outBufLen = 0;
//     GetAdaptersAddresses(AF_UNSPEC, 0, NULL, NULL, &outBufLen);

//     PIP_ADAPTER_ADDRESSES pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);
//     if (GetAdaptersAddresses(AF_UNSPEC, 0, NULL, pAddresses, &outBufLen) != ERROR_SUCCESS) {
//         free(pAddresses);
//         return false;
//     }

//     bool found = false;
//     PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses;
//     while (pCurrAddresses) {
//         // 新增状态检查
//         if (pCurrAddresses->OperStatus == IfOperStatusUp) { // <-- 关键修改
//             QString desc = QString::fromWCharArray(pCurrAddresses->Description);
//             if (desc.contains("TAP", Qt::CaseInsensitive) ||
//                 desc.contains("n2n", Qt::CaseInsensitive)) {
//                 found = true;
//                 break;
//             }
//         }
//         pCurrAddresses = pCurrAddresses->Next;
//     }

//     free(pAddresses);
//     return found;
// }
// void ControlPanelWithoutUserServer::installDriver() {
//     QString driverPath = QCoreApplication::applicationDirPath() + "/driver.exe";

//     // 使用ShellExecuteW以管理员权限运行驱动程序
//     ShellExecuteW(nullptr, L"runas", (LPCWSTR)driverPath.utf16(), nullptr, nullptr, SW_SHOWNORMAL);
// }
// void ControlPanelWithoutUserServer::cleanupNetworkInterfaces() {
//     // // 通过设备管理器卸载 TAP 适配器
//     // DEVINST devInstance;
//     // CONFIGRET cr;

//     // // 使用W版本函数和宽字符串
//     // cr = CM_Locate_DevNodeW(
//     //     &devInstance,
//     //     L"ROOT\\NET\\0000",  // 确保使用L前缀
//     //     CM_LOCATE_DEVNODE_NORMAL
//     //     );

//     // if (cr == CR_SUCCESS) {
//     //     cr = CM_Request_Device_EjectW(  // 使用W版本
//     //         devInstance,
//     //         nullptr,    // 无确认对话框
//     //         nullptr,    // 无回调
//     //         0,          // 无标志
//     //         0           // 无保留
//     //         );
//     //     qDebug() << (cr == CR_SUCCESS ? "移除成功" : "移除失败");
//     // } else {
//     //     qWarning() << "找不到设备，错误码:" << cr;
//     // }
// }
// bool ControlPanelWithoutUserServer::isN2NConnected() {
//     QString ip = getN2NInterfaceIP();
//     // 假设 n2n 分配的是 192.168.100.x 网段
//     return ip.startsWith("192.168.100.") && !ip.isEmpty();
// }
// // 保留原有的网络状态检测方法（hasN2NInterface/isN2NConnected/getN2NInterfaceIP等实现不变）
// // 保留原有的cleanupNetworkInterfaces方法
// QString ControlPanelWithoutUserServer::getN2NInterfaceIP() {
//     ULONG outBufLen = 0;
//     GetAdaptersAddresses(AF_INET, 0, NULL, NULL, &outBufLen);

//     PIP_ADAPTER_ADDRESSES pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);
//     if (GetAdaptersAddresses(AF_INET, 0, NULL, pAddresses, &outBufLen) != ERROR_SUCCESS) {
//         free(pAddresses);
//         return "";
//     }

//     QString ip;
//     PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses;
//     while (pCurrAddresses) {
//         // 新增状态检查：适配器必须处于活动状态
//         if (pCurrAddresses->OperStatus == IfOperStatusUp) { // <-- 关键修改
//             QString desc = QString::fromWCharArray(pCurrAddresses->Description);
//             if (desc.contains("TAP-Windows Adapter V9", Qt::CaseInsensitive)) {
//                 PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pCurrAddresses->FirstUnicastAddress;
//                 while (pUnicast) {
//                     sockaddr_in *sin = (sockaddr_in*)pUnicast->Address.lpSockaddr;
//                     char ipStr[INET_ADDRSTRLEN];
//                     inet_ntop(AF_INET, &sin->sin_addr, ipStr, sizeof(ipStr));
//                     QString currentIp = QString(ipStr);
//                     if (currentIp.startsWith("192.168.100.")) {
//                         free(pAddresses);
//                         return currentIp;
//                     }
//                     pUnicast = pUnicast->Next;
//                 }
//             }
//         }
//         pCurrAddresses = pCurrAddresses->Next;
//     }

//     free(pAddresses);
//     return ip;
// }
// void ControlPanelWithoutUserServer::onDisconnectClicked()
// {
//     if (m_edgeProcess.state() == QProcess::Running) {
//         m_edgeProcess.kill();
//         m_edgeProcess.waitForFinished(1000);
//     }

//     // 清理网络配置
//     if(!m_currentLocalIp.isEmpty()){
//         QString command = QString("Remove-NetIPAddress -IPAddress %1 -ErrorAction SilentlyContinue")
//         .arg(m_currentLocalIp);
//         QProcess::execute("powershell", QStringList() << "-Command" << command);
//         cleanupNetworkInterfaces();
//     }

//     checkN2NStatus();
//     m_currentLocalIp.clear(); // 清空记录的IP
// }

// void ControlPanelWithoutUserServer::checkN2NStatus()
// {
//     bool connected = isN2NConnected() && hasN2NInterface();

//     m_disconnectButton->setEnabled(connected);
//     connectButton->setEnabled(!connected);

//     if(connected){
//         m_statusLabel->setText("connect (" + getN2NInterfaceIP() + ")");
//         m_statusLabel->setStyleSheet("color: green;");
//     }else{
//         m_statusLabel->setText("disconnect");
//         m_statusLabel->setStyleSheet("color: red;");
//     }
// }
