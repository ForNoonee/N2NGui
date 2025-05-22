#include "latencytester.h"
#include <QDebug>
#include<QUdpSocket>
#include<QHostAddress>
#include<QHostInfo>
LatencyTester::LatencyTester(QObject *parent)
    : QObject(parent)
{
}

void LatencyTester::testLatency(const QString &ip)
{
    int latency = performLatencyTest(ip);
    emit latencyResult(ip, latency);
    emit testFinished();
}

int LatencyTester::performLatencyTest(const QString &host)
{

    // 添加DNS解析
    QHostInfo hostInfo = QHostInfo::fromName(host);
    if (hostInfo.error() != QHostInfo::NoError) {
        qWarning() << "DNS lookup failed for" << host << ":" << hostInfo.errorString();
        return -1;
    }
    QHostAddress targetAddress = hostInfo.addresses().first();

    QUdpSocket udpSocket;
    if (!udpSocket.bind(
            QHostAddress(QHostAddress::AnyIPv4), // 地址
            quint16(0),                          // 端口
            QAbstractSocket::DefaultForPlatform  // 绑定模式
            )) {
        qWarning() << "Bind failed:" << udpSocket.errorString();
        return -1;
    }

    QElapsedTimer timer;
    QByteArray datagram = "PING";
    int retry = 0;

    while (retry++ < 3) {
        timer.start();
        udpSocket.writeDatagram(datagram, targetAddress, 7654);

        // 异步等待响应
        if (udpSocket.waitForReadyRead(2000)) {
            qint64 elapsed = timer.elapsed();
            udpSocket.readAll();
            return elapsed;
        }
        qWarning() << "UDP timeout, retry" << retry << "for" << host;
    }
    return -1;
}
