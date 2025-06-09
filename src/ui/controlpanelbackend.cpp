#include "controlpanelbackend.h"
#include <QCoreApplication>
#include <QProcess>
#include <QtConcurrent>
#include <winsock2.h>
#include <Windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <IcmpAPI.h>
#include <cfgmgr32.h>
#include <SetupAPI.h>
#include <initguid.h>   // 必须包含此头文件以初始化 GUID
#include <devguid.h>    // 包含设备类 GUID 的定义
#include <winsvc.h>  // 服务控制头文件
#pragma comment(lib, "SetupAPI.lib")
#pragma comment(lib, "Cfgmgr32.lib")

// ServerInfoModel 实现
int ServerInfoModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : m_servers.size();
}

QVariant ServerInfoModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_servers.size())
        return QVariant();

    const auto &server = m_servers[index.row()];
    const QString ip = server[1];

    switch(role) {
    case NameRole: return server[0];
    case IpRole: return ip;
    case LatencyRole: return m_latencies.value(ip, -1);
    case StatusRole: return m_statuses.value(ip, "Testing...");
    }
    return QVariant();
}

QHash<int, QByteArray> ServerInfoModel::roleNames() const {
    return {
        {NameRole, "name"},
        {IpRole, "ip"},
        {LatencyRole, "latency"},
        {StatusRole, "status"}
    };
}

void ServerInfoModel::updateServer(const QString &ip, int latency, const QString &status) {
    for(int i = 0; i < m_servers.size(); ++i) {
        if(m_servers[i][1] == ip) {
            m_latencies[ip] = latency;
            m_statuses[ip] = status;
            QModelIndex idx = createIndex(i, 0);
            emit dataChanged(idx, idx, {LatencyRole, StatusRole});
            break;
        }
    }
}

void ServerInfoModel::resetData(const QVector<QStringList> &servers) {
    beginResetModel();
    m_servers = servers;
    m_latencies.clear();
    m_statuses.clear();
    endResetModel();

}

// ControlPanelBackend 实现
ControlPanelBackend::ControlPanelBackend(QObject *parent)
    : QObject(parent),
    m_serverModel(new ServerInfoModel(this)),
    m_serverListWatcher(new QFutureWatcher<QStringList>(this)),
    m_latencyTester(new LatencyTester()),
    m_dbHandler("n2n.fornoone.xyz", 5432),
    m_testThread(new QThread(this))
{
    m_latencyTester->moveToThread(m_testThread);
    connect(m_testThread, &QThread::finished, m_latencyTester, &QObject::deleteLater);
    connect(this, &ControlPanelBackend::startLatencyTest, m_latencyTester, &LatencyTester::testLatency);
    connect(m_latencyTester, &LatencyTester::latencyResult, this, &ControlPanelBackend::handleLatencyResult);
    m_testThread->start();

    connect(m_serverListWatcher, &QFutureWatcher<QStringList>::finished, this, &ControlPanelBackend::handleServerListReady);

    m_statusTimer = new QTimer(this);
    connect(m_statusTimer, &QTimer::timeout, this, &ControlPanelBackend::checkN2NStatus);
    m_statusTimer->start(5000);

    connect(&m_edgeProcess, &QProcess::readyReadStandardOutput, this, &ControlPanelBackend::handleProcessOutput);
    connect(&m_edgeProcess, &QProcess::readyReadStandardError, this, &ControlPanelBackend::handleProcessError);

    refreshServerList();
}

ControlPanelBackend::~ControlPanelBackend() {
    if(m_edgeProcess.state() == QProcess::Running) {
        m_edgeProcess.kill();
        m_edgeProcess.waitForFinished();
    }
    m_testThread->quit();
    m_testThread->wait();
}

void ControlPanelBackend::refreshServerList() {
    QFuture<QStringList> future = QtConcurrent::run([this]() {
        return RemoteDatabaseHandler("n2n.fornoone.xyz", 5432).getServerInfo();
    });
    m_serverListWatcher->setFuture(future);
}

void ControlPanelBackend::handleServerListReady() {
    QVector<QStringList> servers;
    QRegularExpression regex(R"(^(.*?)\s*[-(]\s*([^)\s]+)\s*[)]?$)");

    for(const QString &entry : m_serverListWatcher->result()) {
        QRegularExpressionMatch match = regex.match(entry);
        if(match.hasMatch()) {
            const QString ip = match.captured(2).trimmed();
            if(!ip.isEmpty()) {
                servers.append({match.captured(1).trimmed(), ip});
                emit startLatencyTest(ip); // 关键修改：发射信号
            }
        }
    }

    m_serverModel->resetData(servers);
}

void ControlPanelBackend::connectToVpn(const QString &serverIp, const QString &localIp) {
    // 步骤1：先启用网络适配器
    if (!enableNetworkAdapter()) {
        m_connectionStatus = "Error: Cannot enable network adapter";
        emit connectionStatusChanged();
        return; // 启用失败直接终止流程
    }

    // 步骤2：执行原有连接逻辑
    m_currentServerIp = serverIp;
    m_currentIp = localIp;

    if(m_edgeProcess.state() != QProcess::NotRunning) {
        m_edgeProcess.kill();
    }

    QString edgePath = QCoreApplication::applicationDirPath() + "/edge.exe";
    QStringList args = {
        "-c", "mynetwork",
        "-k", "mysecretpass",
        "-a", localIp,
        "-f",
        "-l", serverIp
    };

    // 步骤3：启动VPN连接进程
    m_edgeProcess.start(edgePath, args);

    // 步骤4：更新连接状态
    m_connectionStatus = "Connecting...";
    emit connectionStatusChanged();

    // 可选：添加进程完成回调
    QObject::connect(&m_edgeProcess, &QProcess::finished, [this](int exitCode) {
        if(exitCode == 0) {
            m_connectionStatus = "Connected";
        } else {
            m_connectionStatus = QString("Connection failed (code %1)").arg(exitCode);
        }
        emit connectionStatusChanged();
    });
}
bool ControlPanelBackend::enableNetworkAdapter() {
    HDEVINFO hDevInfo = SetupDiGetClassDevsW(
        &GUID_DEVCLASS_NET,
        nullptr,
        nullptr,
        DIGCF_PRESENT
        );

    if (hDevInfo == INVALID_HANDLE_VALUE) {
        qWarning() << "[启用设备] 无法获取设备列表，错误码:" << GetLastError();
        return false;
    }

    SP_DEVINFO_DATA devInfoData = { sizeof(SP_DEVINFO_DATA) };
    DWORD index = 0;
    bool enabled = false;

    while (SetupDiEnumDeviceInfo(hDevInfo, index++, &devInfoData)) {
        WCHAR deviceName[256] = {0};
        if (!SetupDiGetDeviceRegistryPropertyW(
                hDevInfo,
                &devInfoData,
                SPDRP_DEVICEDESC,
                nullptr,
                (PBYTE)deviceName,
                sizeof(deviceName),
                nullptr
                )) continue;

        QString qDeviceName = QString::fromWCharArray(deviceName);
        if (qDeviceName.contains("TAP-Windows Adapter V9", Qt::CaseInsensitive)) {
            SP_PROPCHANGE_PARAMS params = {0};
            params.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
            params.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
            params.StateChange = DICS_ENABLE;  // 改为启用状态
            params.Scope = DICS_FLAG_GLOBAL;

            if (SetupDiSetClassInstallParamsW(
                    hDevInfo,
                    &devInfoData,
                    (SP_CLASSINSTALL_HEADER*)&params,
                    sizeof(params)
                    )) {
                if (SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hDevInfo, &devInfoData)) {
                    qDebug() << "[启用设备] 设备已启用";
                    enabled = true;
                } else {
                    DWORD err = GetLastError();
                    LPWSTR errorMsg = nullptr;
                    FormatMessageW(
                        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                        nullptr, err, 0, (LPWSTR)&errorMsg, 0, nullptr
                        );
                    qWarning() << "[启用设备] 失败，错误码:" << err
                               << "详情:" << QString::fromWCharArray(errorMsg);
                    LocalFree(errorMsg);
                }
            }
            break;
        }
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);
    return enabled;
}
void ControlPanelBackend::disconnectVpn() {
    if(m_edgeProcess.state() == QProcess::Running) {
        m_edgeProcess.kill();
    }
    cleanupNetworkInterfaces();
    m_connectionStatus = "Disconnected";
    emit connectionStatusChanged();
}

void ControlPanelBackend::installDriver() {
    QString driverPath = QCoreApplication::applicationDirPath() + "/driver.exe";
    ShellExecuteW(nullptr, L"runas", driverPath.toStdWString().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

void ControlPanelBackend::checkN2NStatus() {
    QString newIp = getN2NInterfaceIP();
    bool isConnected = !newIp.isEmpty();

    // 强制更新状态（无论 IP 是否变化）
    if (m_currentIp != newIp) {
        m_currentIp = newIp;
        emit currentIpChanged();
    } else {
        // 防止界面未更新，定期强制通知
        static int refreshCount = 0;
        if (refreshCount++ % 5 == 0) { // 每 25 秒强制刷新一次
            emit currentIpChanged();
        }
    }

    QString newStatus = isConnected ? "Connected" : "Disconnected";
    if (m_connectionStatus != newStatus) {
        m_connectionStatus = newStatus;
        emit connectionStatusChanged();
    }
}

void ControlPanelBackend::handleLatencyResult(const QString &ip, int latency) {
    m_serverModel->updateServer(ip, latency, latency > 0 ? QString("%1 ms").arg(latency) : "Offline");
}

bool ControlPanelBackend::hasN2NInterface() {
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

QString ControlPanelBackend::getN2NInterfaceIP() {
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
                    if (currentIp.startsWith("192.166.")) {
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
bool EnableDriverPrivileges() {
    HANDLE hToken;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        return false;
    }

    TOKEN_PRIVILEGES tp;
    LUID luid;
    if (!LookupPrivilegeValueW(nullptr, SE_LOAD_DRIVER_NAME, &luid)) {
        CloseHandle(hToken);
        return false;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), nullptr, nullptr)) {
        CloseHandle(hToken);
        return false;
    }

    CloseHandle(hToken);
    return true;
}
void ControlPanelBackend::cleanupNetworkInterfaces() {
    // 获取所有网络设备
    HDEVINFO hDevInfo = SetupDiGetClassDevsW(
        &GUID_DEVCLASS_NET,
        nullptr,
        nullptr,
        DIGCF_PRESENT
        );

    if (hDevInfo == INVALID_HANDLE_VALUE) {
        qWarning() << "无法获取设备列表，错误码:" << GetLastError();
        return;
    }

    SP_DEVINFO_DATA devInfoData = { sizeof(SP_DEVINFO_DATA) };
    DWORD index = 0;
    bool disabled = false;

    while (SetupDiEnumDeviceInfo(hDevInfo, index++, &devInfoData)) {
        WCHAR deviceName[256] = {0};

        // 获取设备名称
        if (!SetupDiGetDeviceRegistryPropertyW(
                hDevInfo,
                &devInfoData,
                SPDRP_DEVICEDESC,
                nullptr,
                (PBYTE)deviceName,
                sizeof(deviceName),
                nullptr
                )) {
            continue;
        }

        QString qDeviceName = QString::fromWCharArray(deviceName);
        if (qDeviceName.contains("TAP-Windows Adapter V9", Qt::CaseInsensitive)) {
            // 准备禁用设备参数
            SP_PROPCHANGE_PARAMS params = {0};
            params.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
            params.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
            params.StateChange = DICS_DISABLE;      // 禁用设备
            params.Scope = DICS_FLAG_GLOBAL;        // 全局生效（而非仅当前配置）
            params.HwProfile = 0;                   // 所有硬件配置文件

            // 设置参数并执行禁用操作
            if (SetupDiSetClassInstallParamsW(
                    hDevInfo,
                    &devInfoData,
                    (SP_CLASSINSTALL_HEADER*)&params,
                    sizeof(params)
                    )) {
                if (SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hDevInfo, &devInfoData)) {
                    qDebug() << "设备禁用成功";
                    disabled = true;
                } else {
                    DWORD err = GetLastError();
                    LPWSTR errorMsg = nullptr;
                    FormatMessageW(
                        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                        nullptr, err, 0, (LPWSTR)&errorMsg, 0, nullptr
                        );
                    qWarning() << "禁用失败，错误码:" << err
                               << "详情:" << QString::fromWCharArray(errorMsg);
                    LocalFree(errorMsg);
                }
            }
            break;
        }
    }

    if (!disabled) {
        qWarning() << "未找到目标设备或禁用失败";
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);
 }
void ControlPanelBackend::appendLog(const QString &log) {
    m_logs += log + "\n";
    emit logsChanged();
}

void ControlPanelBackend::handleProcessOutput() {
    appendLog(m_edgeProcess.readAllStandardOutput());
}

void ControlPanelBackend::handleProcessError() {
    appendLog("[ERROR] " + m_edgeProcess.readAllStandardError());
}
