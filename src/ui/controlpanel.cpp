#include "controlpanel.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QFileInfo>
#include <QMessageBox>
#include <QCoreApplication>
#include <QProcess>
#include "RemoteDatabaseHandler.h"
#include <winsock2.h>
#include <Windows.h>       // 包含Winsock
#include <ws2tcpip.h>        // 包含TCP/IP协议族的affinity
#include <iphlpapi.h>        // 包含IP Helper API
#include <IcmpAPI.h>         // 包含ICMP API
#include <IcmpAPI.h>

ControlPanel::ControlPanel(QWidget *parent)
    : QMainWindow(parent), serverListWatcher(new QFutureWatcher<QStringList>(this))
{
    setupUI();
    refreshServerList();
    startConnectionTest();
}

void ControlPanel::setupUI()
{
    serverTable = new QTableWidget(this);
    serverTable->setColumnCount(3);
    serverTable->setHorizontalHeaderLabels({"Server", "IP", "Status"});
    serverTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    serverTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    latencyLabel = new QLabel("Latency: -- ms", this);
    speedLabel = new QLabel("Speed: -- Mbps", this);
    driverButton = new QPushButton("Install Driver", this);
    connectButton = new QPushButton("Connect VPN", this);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(driverButton);
    buttonLayout->addWidget(connectButton);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addWidget(serverTable);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(latencyLabel);
    ipAddressInput = new QLineEdit(this);
    ipAddressInput->setPlaceholderText("请输入IP地址");
    ipAddressInput->setText("192.168.100.1");
    mainLayout->addWidget(ipAddressInput);
    mainLayout->addWidget(speedLabel);

    QWidget *centralWidget = new QWidget();
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    connect(driverButton, &QPushButton::clicked, this, &ControlPanel::installDriver);
    connect(connectButton, &QPushButton::clicked, this, &ControlPanel::connectToVpn);

    connect(serverListWatcher, &QFutureWatcher<QStringList>::finished, this, &ControlPanel::handleServerListReady);

    QTimer *timer = new QTimer(this);

    // 连接定时器的timeout信号到你的槽函数
    connect(timer, &QTimer::timeout, this, &ControlPanel::refreshServerList);

    // 设置定时器每5秒触发一次（5000毫秒）
    timer->start(5000);
}
void ControlPanel::updateServerList() {
    refreshServerList();
}
void ControlPanel::installDriver() {
    QString driverPath = QCoreApplication::applicationDirPath() + "/driver.exe";

    // 使用ShellExecuteW以管理员权限运行驱动程序
    ShellExecuteW(nullptr, L"runas", (LPCWSTR)driverPath.utf16(), nullptr, nullptr, SW_SHOWNORMAL);
}

void ControlPanel::connectToVpn() {
    int currentRow = serverTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "warnning", "chose a server first");
        return;
    }
    QString ServerAddress = serverTable->item(currentRow, 1)->text();
    QString ipAddress = ipAddressInput->text();

    // 获取应用程序的目录路径
    QString appDir = QCoreApplication::applicationDirPath();
    QString edgePath = appDir + "\\edge.exe"; // 确保 edge.exe 在该目录下

    // 构建命令行参数
    QString arguments = QString("-c mynetwork -k mysecretpass -a %1 -f -l %2")
                            .arg(ipAddress)
                            .arg(ServerAddress);

    // 将 QString 转换为 LPCWSTR
    LPCWSTR lpFile = (LPCWSTR)edgePath.utf16();
    LPCWSTR lpParameters = (LPCWSTR)arguments.utf16();

    // 使用 ShellExecuteW 以管理员权限运行 edge.exe
    HINSTANCE result = ShellExecuteW(
        nullptr,           // 父窗口句柄
        L"runas",          // 操作类型，"runas" 表示以管理员权限运行
        lpFile,            // 可执行文件路径
        lpParameters,      // 参数
        NULL,              // 默认目录
        SW_SHOWNORMAL      // 显示窗口方式
        );

    // 检查 ShellExecuteW 的返回值
    if ((INT_PTR)result <= 32) {
        // 处理错误，例如：
        qDebug() << "Failed to execute edge.exe. Error code:" << (INT_PTR)result;
    }
}
void ControlPanel::refreshServerList()
{
    // Start the asynchronous task
    QFuture<QStringList> future = QtConcurrent::run([this]() -> QStringList {
        QStringList servers;
        RemoteDatabaseHandler temp("n2n.fornoone.xyz", 5432);
        servers = temp.getServerInfo();

        // Iterate through each server and perform ping
        for (int i = 0; i < servers.size(); ++i) {
            QStringList parts = servers[i].split(", "); // Expected format: "Name, IP"
            if (parts.size() >= 2) {
                QString serverName = parts[0];
                QString ipAddress = parts[1];

                // Perform ping to get latency
                int latency = pingServer(ipAddress);

                // Update the server info with latency
                if (latency != -1) {
                    servers[i] = QString("%1, %2, %3 ms").arg(serverName, ipAddress, QString::number(latency));
                } else {
                    servers[i] = QString("%1, %2, Offline").arg(serverName, ipAddress);
                }
            }
        }

        return servers;
    });

    // Assign the future to the watcher
    serverListWatcher->setFuture(future);
}
void ControlPanel::handleServerListReady()
{
    QStringList servers = serverListWatcher->result();

    serverTable->setRowCount(servers.size());

    for (int i = 0; i < servers.size(); ++i) {
        QStringList parts = servers[i].split(", "); // Expected format: "Name, IP, Status"

        if (parts.size() >= 3) {
            serverTable->setItem(i, 0, new QTableWidgetItem(parts[0])); // Server Name
            serverTable->setItem(i, 1, new QTableWidgetItem(parts[1])); // IP Address
            serverTable->setItem(i, 2, new QTableWidgetItem(parts[2])); // Status (Latency or Offline)
        }
    }

    // Optionally, you can sort the table or perform other UI updates here
}
void ControlPanel::startConnectionTest()
{
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout,
            this, &ControlPanel::updateNetworkStatus);
    updateTimer->start(5000); // 每5秒更新一次
}

void ControlPanel::updateNetworkStatus()
{
    int latency = QRandomGenerator::global()->bounded(10, 100);
    int speed = QRandomGenerator::global()->bounded(50, 500);
    latencyLabel->setText(QString("Latency: %1 ms").arg(latency));
    speedLabel->setText(QString("Speed: %1 Mbps").arg(speed));
}

int ControlPanel::pingServer(const QString &hostName) {
    // 初始化 Winsock
    WSADATA wsaData;
    int wsaInit = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaInit != 0) {
        qDebug() << "WSAStartup failed with error:" << wsaInit;
        return -1;
    }

    // 解析主机名到 IP 地址
    struct addrinfo hints;
    struct addrinfo *result = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // 仅IPv4
    hints.ai_socktype = SOCK_RAW;
    hints.ai_protocol = IPPROTO_ICMP;

    int addrResult = getaddrinfo(hostName.toStdString().c_str(), NULL, &hints, &result);
    if (addrResult != 0) {
        qDebug() << "getaddrinfo failed for host" << hostName << "with error:" << addrResult;
        WSACleanup();
        return -1;
    }

    struct sockaddr_in *ipv4 = (struct sockaddr_in *)result->ai_addr;
    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(ipv4->sin_addr), ipStr, INET_ADDRSTRLEN);

    QString ipAddress = QString::fromUtf8(ipStr);
    qDebug() << "Resolved IP address for host" << hostName << ":" << ipAddress;

    freeaddrinfo(result); // 释放资源

    // 创建 ICMP 文件句柄
    HANDLE hIcmpFile = IcmpCreateFile();
    if (hIcmpFile == INVALID_HANDLE_VALUE) {
        qDebug() << "IcmpCreateFile failed";
        WSACleanup();
        return -1;
    }

    // 准备发送的数据
    BYTE sendData[32];
    memset(sendData, 'E', sizeof(sendData));

    // 计算回复缓冲区大小
    DWORD replySize = sizeof(ICMP_ECHO_REPLY) + sizeof(sendData);
    LPVOID replyBuffer = malloc(replySize);
    if (replyBuffer == NULL) {
        qDebug() << "Failed to allocate memory for replyBuffer";
        IcmpCloseHandle(hIcmpFile);
        WSACleanup();
        return -1;
    }

    // 将 IP 地址转换为 DWORD
    DWORD ipAddr = inet_addr(ipAddress.toStdString().c_str());
    if (ipAddr == INADDR_NONE) {
        qDebug() << "inet_addr failed to convert IP address:" << ipAddress;
        free(replyBuffer);
        IcmpCloseHandle(hIcmpFile);
        WSACleanup();
        return -1;
    }

    // 发送 ICMP 请求
    DWORD dwRetVal = IcmpSendEcho(
        hIcmpFile,
        ipAddr,
        sendData,
        sizeof(sendData),
        NULL,
        replyBuffer,
        replySize,
        1000 // 超时1秒
        );

    DWORD latency = -1;
    if (dwRetVal != 0) {
        PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)replyBuffer;
        if (pEchoReply->Status == IP_SUCCESS) {
            latency = pEchoReply->RoundTripTime;
            qDebug() << "Ping to" << hostName << "successful. RoundTripTime:" << latency << "ms";
        } else {
            qDebug() << "Ping to" << hostName << "failed with status:" << pEchoReply->Status;
        }
    } else {
        DWORD dwError = GetLastError();
        qDebug() << "Ping failed for" << hostName << "with error" << dwError;
    }

    // 清理资源
    free(replyBuffer);
    IcmpCloseHandle(hIcmpFile);
    WSACleanup();

    return static_cast<int>(latency);
}
