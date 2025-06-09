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
#include <QProcess>
#include <QPlainTextEdit>  // 新增日志组件
#include <QDialog>         // 日志窗口
#include <cfgmgr32.h> // 设备管理API
#include"latencytester.h"
#pragma comment(lib, "Cfgmgr32.lib")  // MSVC专用链接指令
ControlPanel::ControlPanel(QWidget *parent)
    : QMainWindow(parent), serverListWatcher(new QFutureWatcher<QStringList>(this)),m_latencyTester(new LatencyTester()),
    m_testThread(new QThread(this))
{
    setupUI();
    init();
}
void ControlPanel::init(){
    refreshServerList();
    startConnectionTest();
    m_latencyTester->moveToThread(m_testThread);
    // 连接信号槽
    connect(m_testThread, &QThread::finished,
            m_latencyTester, &QObject::deleteLater);
    connect(this, &ControlPanel::startLatencyTest,
            m_latencyTester, &LatencyTester::testLatency);
    connect(m_latencyTester, &LatencyTester::latencyResult,
            this, &ControlPanel::handleLatencyResult);
    m_testThread->start();
}
ControlPanel::~ControlPanel() {
    // 强制终止 edge 进程
    if (m_edgeProcess.state() == QProcess::Running) {
        m_edgeProcess.kill();
        m_edgeProcess.waitForFinished(1000);
    }

    // 调用系统级清理
    QString command = QString("Remove-NetIPAddress -IPAddress %1 -ErrorAction SilentlyContinue").arg(m_currentServerIp);
    QProcess::execute("powershell", QStringList() << "-Command" << command);
    m_testThread->quit();
    m_testThread->wait();
}

void ControlPanel::cleanupNetworkInterfaces() {
    // // 通过设备管理器卸载 TAP 适配器
    // DEVINST devInstance;
    // CONFIGRET cr;

    // // 使用W版本函数和宽字符串
    // cr = CM_Locate_DevNodeW(
    //     &devInstance,
    //     L"ROOT\\NET\\0000",  // 确保使用L前缀
    //     CM_LOCATE_DEVNODE_NORMAL
    //     );

    // if (cr == CR_SUCCESS) {
    //     cr = CM_Request_Device_EjectW(  // 使用W版本
    //         devInstance,
    //         nullptr,    // 无确认对话框
    //         nullptr,    // 无回调
    //         0,          // 无标志
    //         0           // 无保留
    //         );
    //     qDebug() << (cr == CR_SUCCESS ? "移除成功" : "移除失败");
    // } else {
    //     qWarning() << "找不到设备，错误码:" << cr;
    // }
}
bool ControlPanel::hasN2NInterface() {
    ULONG outBufLen = 0;
    GetAdaptersAddresses(AF_UNSPEC, 0, NULL, NULL, &outBufLen);

    PIP_ADAPTER_ADDRESSES pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);
    if (GetAdaptersAddresses(AF_UNSPEC, 0, NULL, pAddresses, &outBufLen) != ERROR_SUCCESS) {
        free(pAddresses);
        return false;
    }

    bool found = false;
    PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses;
    while (pCurrAddresses) {
        // 新增状态检查
        if (pCurrAddresses->OperStatus == IfOperStatusUp) { // <-- 关键修改
            QString desc = QString::fromWCharArray(pCurrAddresses->Description);
            if (desc.contains("TAP", Qt::CaseInsensitive) ||
                desc.contains("n2n", Qt::CaseInsensitive)) {
                found = true;
                break;
            }
        }
        pCurrAddresses = pCurrAddresses->Next;
    }

    free(pAddresses);
    return found;
}
bool ControlPanel::isN2NConnected() {
    QString ip = getN2NInterfaceIP();
    // 假设 n2n 分配的是 192.168.100.x 网段
    return ip.startsWith("192.168.100.") && !ip.isEmpty();
}
QString ControlPanel::getN2NInterfaceIP() {
    ULONG outBufLen = 0;
    GetAdaptersAddresses(AF_INET, 0, NULL, NULL, &outBufLen);

    PIP_ADAPTER_ADDRESSES pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);
    if (GetAdaptersAddresses(AF_INET, 0, NULL, pAddresses, &outBufLen) != ERROR_SUCCESS) {
        free(pAddresses);
        return "";
    }

    QString ip;
    PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses;
    while (pCurrAddresses) {
        // 新增状态检查：适配器必须处于活动状态
        if (pCurrAddresses->OperStatus == IfOperStatusUp) { // <-- 关键修改
            QString desc = QString::fromWCharArray(pCurrAddresses->Description);
            if (desc.contains("TAP-Windows Adapter V9", Qt::CaseInsensitive)) {
                PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pCurrAddresses->FirstUnicastAddress;
                while (pUnicast) {
                    sockaddr_in *sin = (sockaddr_in*)pUnicast->Address.lpSockaddr;
                    char ipStr[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &sin->sin_addr, ipStr, sizeof(ipStr));
                    QString currentIp = QString(ipStr);
                    if (currentIp.startsWith("192.168.100.")) {
                        free(pAddresses);
                        return currentIp;
                    }
                    pUnicast = pUnicast->Next;
                }
            }
        }
        pCurrAddresses = pCurrAddresses->Next;
    }

    free(pAddresses);
    return ip;
}
void ControlPanel::setupUI()
{
    serverTable = new QTableWidget(this);
    serverTable->setColumnCount(3);
    serverTable->setHorizontalHeaderLabels({"Server", "IP", "Status"});
    serverTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    serverTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    m_latencyWatcher= new QFutureWatcher<int>();

    latencyLabel = new QLabel("Latency: -- ms", this);
    // speedLabel = new QLabel("Speed: -- Mbps", this);
    driverButton = new QPushButton("Install Driver", this);
    connectButton = new QPushButton("Connect VPN", this);
    connectButton = new QPushButton("Connect VPN", this);
    m_disconnectButton = new QPushButton("Disconnect", this);
    m_disconnectButton->setEnabled(false); // 初始禁用

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(driverButton);
    buttonLayout->addWidget(connectButton);
     buttonLayout->addWidget(m_disconnectButton); // 新增断开按钮

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addWidget(serverTable);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(latencyLabel);
    ipAddressInput = new QLineEdit(this);
    ipAddressInput->setPlaceholderText("请输入IP地址");
    ipAddressInput->setText("192.168.100.1");
    mainLayout->addWidget(ipAddressInput);
    // mainLayout->addWidget(speedLabel);

    QWidget *centralWidget = new QWidget();
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    connect(driverButton, &QPushButton::clicked, this, &ControlPanel::installDriver);
    connect(connectButton, &QPushButton::clicked, this, &ControlPanel::connectToVpn);

    connect(serverListWatcher, &QFutureWatcher<QStringList>::finished, this, &ControlPanel::handleServerListReady);
    connect(m_disconnectButton, &QPushButton::clicked,
            this, &ControlPanel::onDisconnectClicked);
    QTimer *timer = new QTimer(this);

    // 连接定时器的timeout信号到你的槽函数
    connect(timer, &QTimer::timeout, this, &ControlPanel::refreshServerList);

    // 设置定时器每15秒触发一次（5000毫秒）
    timer->start(30000);
    // 新增状态标签
    m_statusLabel = new QLabel("VPN Status: Disconnected", this);
    m_statusLabel->setStyleSheet("color: red; font-weight: bold;");
    mainLayout->addWidget(m_statusLabel);

    // 新增日志按钮
    QPushButton *logButton = new QPushButton("View Logs", this);
    buttonLayout->addWidget(logButton);  // 将日志按钮添加到按钮布局

    // 连接信号
    connect(logButton, &QPushButton::clicked, this, &ControlPanel::showLogWindow);

    // 初始化日志窗口
    m_logDialog = new QDialog(this);
    m_logViewer = new QPlainTextEdit(m_logDialog);
    m_logViewer->setReadOnly(true);
    QVBoxLayout *logLayout = new QVBoxLayout(m_logDialog);
    logLayout->addWidget(m_logViewer);
    m_logDialog->setWindowTitle("VPN Logs");
    m_logDialog->resize(600, 400);

    // 初始化状态检测定时器
    m_statusTimer = new QTimer(this);
    connect(m_statusTimer, &QTimer::timeout, this, &ControlPanel::checkN2NStatus);
    m_statusTimer->start(5000);  // 30秒检测一次
}
void ControlPanel::updateServerList() {
    refreshServerList();
}
void ControlPanel::onDisconnectClicked() {
    if (m_edgeProcess.state() == QProcess::Running) {
        // 明确指定重载版本
        connect(&m_edgeProcess,
                QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this,
                [this](int exitCode, QProcess::ExitStatus exitStatus) {
                    Q_UNUSED(exitCode);
                    Q_UNUSED(exitStatus);
                    handleDisconnectCompleted();
                });

        m_edgeProcess.terminate();
        QTimer::singleShot(2000, this, [this]() {
            if (m_edgeProcess.state() == QProcess::Running) {
                m_edgeProcess.kill();
            }
        });
    } else {
        handleDisconnectCompleted();
    }
}

void ControlPanel::handleDisconnectCompleted() {
    if (!m_currentLocalIp.isEmpty()) {
        QString command = QString(
                              "Start-Process powershell -Verb RunAs -ArgumentList '-Command \"Remove-NetIPAddress -IPAddress %1 -ErrorAction SilentlyContinue\"'"
                              ).arg(m_currentLocalIp);

        QProcess *cleanupProcess = new QProcess(this);
        connect(cleanupProcess,
                QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this,
                [this, cleanupProcess](int exitCode, QProcess::ExitStatus exitStatus) {
                    Q_UNUSED(exitCode);
                    Q_UNUSED(exitStatus);
                    cleanupProcess->deleteLater();
                    cleanupNetworkInterfaces();
                    checkN2NStatus();
                });
        cleanupProcess->start("cmd.exe", QStringList() << "/C" << command);
    } else {
        cleanupNetworkInterfaces();
        checkN2NStatus();
    }
}
void ControlPanel::checkN2NStatus() {
    bool isProcessRunning = (m_edgeProcess.state() == QProcess::Running);
    bool hasInterface = hasN2NInterface();
    bool isConnected = isN2NConnected();

    // 只有同时满足以下条件才认为已连接
    bool realConnected = isProcessRunning && hasInterface && isConnected;

    QString statusText;
    if (realConnected) {
        statusText = "VPN Status: Connected (IP: " + getN2NInterfaceIP() + ")";
        m_statusLabel->setStyleSheet("color: green; font-weight: bold;");
    } else if (hasInterface) {
        statusText = "VPN Status: Interface Found (No Connection)";
        m_statusLabel->setStyleSheet("color: orange; font-weight: bold;");
    } else {
        statusText = "VPN Status: Disconnected";
        m_statusLabel->setStyleSheet("color: red; font-weight: bold;");
    }

    m_statusLabel->setText(statusText);
    m_disconnectButton->setEnabled(realConnected);
    connectButton->setEnabled(!realConnected);
}
void ControlPanel::installDriver() {
    QString driverPath = QCoreApplication::applicationDirPath() + "/driver.exe";

    // 使用ShellExecuteW以管理员权限运行驱动程序
    ShellExecuteW(nullptr, L"runas", (LPCWSTR)driverPath.utf16(), nullptr, nullptr, SW_SHOWNORMAL);
}

void ControlPanel::connectToVpn() {
    int currentRow = serverTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Warning", "Choose a server first.");
        return;
    }
    m_currentServerIp = serverTable->item(currentRow, 1)->text();
    m_currentLocalIp=ipAddressInput->text();
    QString serverAddress = serverTable->item(currentRow, 1)->text();
    QString ipAddress = ipAddressInput->text();

    // 停止已有进程
    if (m_edgeProcess.state() != QProcess::NotRunning) {
        m_edgeProcess.terminate();
        m_edgeProcess.waitForFinished();
    }

    // 设置进程工作目录
    QString edgePath = QCoreApplication::applicationDirPath() + "/edge.exe";
    m_edgeProcess.setWorkingDirectory(QCoreApplication::applicationDirPath());

    // 绑定日志捕获
    connect(&m_edgeProcess, &QProcess::readyReadStandardOutput, [this]() {
        m_logViewer->appendPlainText(m_edgeProcess.readAllStandardOutput());
    });
    connect(&m_edgeProcess, &QProcess::readyReadStandardError, [this]() {
        m_logViewer->appendPlainText("[ERROR] " + m_edgeProcess.readAllStandardError());
    });
    // 新增：连接前重置状态
    m_statusLabel->setText("VPN Status: Connecting...");
    m_statusLabel->setStyleSheet("color: blue; font-weight: bold;");
    m_disconnectButton->setEnabled(false);
    connectButton->setEnabled(false);
    // 启动进程
    QStringList args = {
        "-c", "mynetwork",
        "-k", "mysecretpass",
        "-a", ipAddress,
        "-f",
        "-l", serverAddress
    };
    m_edgeProcess.start(edgePath, args);

    // 处理启动失败
    if (!m_edgeProcess.waitForStarted()) {
        QMessageBox::critical(this, "Error", "Failed to start edge.exe");
    }
    if (m_edgeProcess.state() == QProcess::Running) {
        connectButton->setEnabled(false);
        m_disconnectButton->setEnabled(true);
    }
}

// 显示日志窗口
void ControlPanel::showLogWindow() {
    m_logDialog->show();
}

void ControlPanel::refreshServerList()
{
    QFuture<QStringList> future = QtConcurrent::run([this]() -> QStringList {
        QStringList servers;
        RemoteDatabaseHandler temp("n2n.fornoone.xyz", 5432);
        servers = temp.getServerInfo();
        return servers; // 不再在此处测试延迟
    });
    serverListWatcher->setFuture(future);
}
void ControlPanel::handleServerListReady()
{
    QStringList servers = serverListWatcher->result();
    serverTable->setRowCount(servers.size());
    m_serverStatusItems.clear();

    QRegularExpression regex(R"(^(.*?)\s*[-(]\s*([^)\s]+)\s*[)]?$)");
    regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);

    for (int i = 0; i < servers.size(); ++i) {
        QString serverInfo = servers[i].trimmed();
        QRegularExpressionMatch match = regex.match(serverInfo);

        QString serverName, ipAddress;
        bool isValid = false;

        if (match.hasMatch()) {
            serverName = match.captured(1).trimmed();
            ipAddress = match.captured(2).trimmed();
            isValid = !serverName.isEmpty() && isValidAddress(ipAddress);
        }

        if (!isValid) {
            qWarning() << "无效或格式错误的服务器条目:" << serverInfo;
            serverName = "格式错误";
            ipAddress = "N/A";
        }

        QTableWidgetItem* statusItem = new QTableWidgetItem(isValid ? "Testing..." : "Invalid");
        statusItem->setForeground(isValid ? Qt::gray : Qt::red);

        serverTable->setItem(i, 0, new QTableWidgetItem(serverName));
        serverTable->setItem(i, 1, new QTableWidgetItem(ipAddress));
        serverTable->setItem(i, 2, statusItem);

        if (isValid) {
            m_serverStatusItems[ipAddress] = statusItem;
            emit startLatencyTest(ipAddress);
        }
    }
}
bool ControlPanel::isValidAddress(const QString& address) const {
    QRegularExpression ipRegex(R"(^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$)");
    QRegularExpression domainRegex(R"(^([a-zA-Z0-9-]+\.)+[a-zA-Z]{2,}$)");

    return ipRegex.match(address).hasMatch() ||
           domainRegex.match(address).hasMatch();
}
void ControlPanel::handleLatencyResult(const QString &ip, int latency)
{
    if (latency == -1) {
        // 3次重试逻辑
        static QMap<QString, int> retryCount;
        if (retryCount[ip]++ < 3) {
            QTimer::singleShot(2000, [this, ip](){
                emit startLatencyTest(ip);
            });
            return;
        }
    }
    if (!m_serverStatusItems.contains(ip)) return;

    QTableWidgetItem* item = m_serverStatusItems[ip];
    if (latency > 0) {
        item->setText(QString("%1 ms").arg(latency));
        item->setForeground(latency < 100 ? Qt::darkGreen : Qt::darkRed);
    } else {
        item->setText("Offline");
        item->setForeground(Qt::red);
    }
}
void ControlPanel::startConnectionTest() {
    // 替换原有随机延迟逻辑
      m_statusTimer->start(30000);
}
void ControlPanel::updateNetworkStatus()
{
    int latency = QRandomGenerator::global()->bounded(10, 100);
    int speed = QRandomGenerator::global()->bounded(50, 500);
    latencyLabel->setText(QString("Latency: %1 ms").arg(latency));
    speedLabel->setText(QString("Speed: %1 Mbps").arg(speed));
}

