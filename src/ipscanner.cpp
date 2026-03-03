#include "ipscanner.h"
#include "ipscannerthread.h"

#include <QHostAddress>
#include <QProcess>
#include <QCoreApplication>
#include <QThread>
#include <QDebug>
#include <QMutexLocker>
#include <QMutex>
#include <QAtomicInt>

IPScanner::IPScanner(QObject *parent)
    : QObject(parent)
    , m_startIp(1)
    , m_endIp(254)
    , m_currentIp(0)
    , m_totalIps(0)
    , m_scannedCount(0)
    , m_stopRequested(0)
    , m_isScanning(false)
{
    m_threadPool = new QThreadPool(this);
    m_threadPool->setMaxThreadCount(QThread::idealThreadCount() * 2);

    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, [this]() {
        if (m_isScanning) {
            emit scanProgress(m_scannedCount.loadRelaxed(), m_totalIps);
        }
    });
    m_updateTimer->start(100);
}

IPScanner::~IPScanner()
{
    stop();
    m_threadPool->clear();
    m_threadPool->waitForDone(3000);
}

void IPScanner::startScan(const QString &baseIp, int startIp, int endIp)
{
    if (m_isScanning) {
        return;
    }

    m_baseIp = baseIp;
    m_startIp = qBound(1, startIp, 254);
    m_endIp = qBound(1, endIp, 254);

    if (m_startIp > m_endIp) {
        qSwap(m_startIp, m_endIp);
    }

    m_totalIps = m_endIp - m_startIp + 1;
    m_currentIp = m_startIp;
    m_scannedCount = 0;
    m_stopRequested = 0;
    m_isScanning = true;

    m_elapsedTimer.start();

    // Create and run all scan threads
    for (int i = m_startIp; i <= m_endIp; ++i) {
        IPScannerThread *worker = new IPScannerThread(QString("%1.%2").arg(m_baseIp).arg(i));
        worker->setAutoDelete(true);

        connect(worker, &IPScannerThread::scanComplete, this,
            [this](const DeviceInfo &info) {
                m_scannedCount.fetchAndAddRelaxed(1);
                emit scanResult(info);

                if (m_scannedCount.loadRelaxed() >= m_totalIps || m_stopRequested.loadRelaxed()) {
                    onScanComplete();
                }
            });

        m_threadPool->start(worker);
    }
}

void IPScanner::stop()
{
    if (m_isScanning) {
        m_stopRequested.storeRelaxed(1);
        m_threadPool->clear();
    }
}

void IPScanner::onScanComplete()
{
    if (m_isScanning) {
        m_isScanning = false;
        emit scanProgress(m_totalIps, m_totalIps);
        emit scanFinished();
    }
}
