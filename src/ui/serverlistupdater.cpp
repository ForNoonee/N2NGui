#include "ServerListUpdater.h"
#include "RemoteDatabaseHandler.h"
#include <QDebug>
#include <winsock2.h>
#include <Windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <IcmpAPI.h>

// Constructor
ServerListUpdater::ServerListUpdater(const QString &dbHost, int dbPort, QObject *parent)
    : QObject(parent), dbHost(dbHost), dbPort(dbPort)
{
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &ServerListUpdater::performUpdate);
}

// Destructor
ServerListUpdater::~ServerListUpdater()
{
    stopUpdating();
}

// 开始定时更新
void ServerListUpdater::startUpdating()
{
    performUpdate(); // 初次更新
    timer->start(3000); // 每3秒更新一次
}

// 停止定时更新
void ServerListUpdater::stopUpdating()
{
    if (timer->isActive()) {
        timer->stop();
    }
}

// 执行更新操作
void ServerListUpdater::performUpdate()
{
    // RemoteDatabaseHandler temp(dbHost, dbPort);
    // QStringList servers = temp.getServerInfo();
    // if (servers.isEmpty()) {
    //     emit errorOccurred("");
    //     return;
    // }

    // QList<ServerInfo> serverInfoList;
    // for (const QString &server : servers) {
    //     QStringList parts = server.split(", ");
    //     if (parts.size() >= 2) {
    //         ServerInfo info;
    //         info.name = parts[0];
    //         info.ip = parts[1];
    //         int latency = pingServer(info.ip);
    //         if (latency != -1) {
    //             info.status = QString::number(latency) + " ms";
    //         } else {
    //             info.status = "Offline";
    //         }
    //         serverInfoList.append(info);
    //     }
    // }

    // emit serverListUpdated(serverInfoList);
}

// 进行 ping 测试，返回延迟时间（ms）或 -1 表示失败
int ServerListUpdater::pingServer(const QString &hostName)
{
    // 初始化 Winsock
    WSADATA wsaData;
    int wsaInit = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaInit != 0) {
        qDebug() << "WSAStartup failed with error:" << wsaInit;
        return -1;
    }

    // 解析主机名到 IP 地址
    struct addrinfo hints;
    struct addrinfo *result = nullptr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // 仅IPv4
    hints.ai_socktype = SOCK_RAW;
    hints.ai_protocol = IPPROTO_ICMP;

    int addrResult = getaddrinfo(hostName.toStdString().c_str(), nullptr, &hints, &result);
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
    if (replyBuffer == nullptr) {
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
        nullptr,
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
