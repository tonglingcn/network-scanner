#ifndef IPSCANNER_H
#define IPSCANNER_H

#include <QObject>
#include <QString>
#include <QFuture>
#include <QThreadPool>
#include <QRunnable>
#include <QTimer>
#include <QAtomicInt>
#include <QElapsedTimer>
#include "deviceinfo.h"

class IPScanner : public QObject
{
    Q_OBJECT

public:
    explicit IPScanner(QObject *parent = nullptr);
    ~IPScanner();

    void startScan(const QString &baseIp, int startIp, int endIp);
    void stop();
    bool isScanning() const { return m_isScanning; }

signals:
    void scanProgress(int current, int total);
    void scanResult(const DeviceInfo &info);
    void scanFinished();

private slots:
    void onScanComplete();

private:
    QString m_baseIp;
    int m_startIp;
    int m_endIp;
    int m_currentIp;
    int m_totalIps;
    QAtomicInt m_scannedCount;
    QAtomicInt m_stopRequested;
    QThreadPool *m_threadPool;
    QTimer *m_updateTimer;
    QElapsedTimer m_elapsedTimer;
    bool m_isScanning;
};

#endif // IPSCANNER_H
