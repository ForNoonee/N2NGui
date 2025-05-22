#ifndef LATENCYTESTER_H
#define LATENCYTESTER_H

#include <QObject>
#include <QThread>
#include <QString>
#include <QProcess>

class LatencyTester : public QObject
{
    Q_OBJECT
public:
    explicit LatencyTester(QObject *parent = nullptr);

public slots:
    void testLatency(const QString &ip);

signals:
    void latencyResult(const QString &ip, int latencyMs);
    void testFinished();

private:
    int performLatencyTest(const QString &ip);
};

#endif // LATENCYTESTER_H
