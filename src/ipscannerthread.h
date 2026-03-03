#ifndef IPSCANNERTHREAD_H
#define IPSCANNERTHREAD_H

#include <QObject>
#include <QRunnable>
#include <QString>
#include <QHostAddress>
#include <QProcess>
#include "deviceinfo.h"

class IPScannerThread : public QObject, public QRunnable
{
    Q_OBJECT

public:
    explicit IPScannerThread(const QString &ip, QObject *parent = nullptr);
    ~IPScannerThread();

    void run() override;

signals:
    void scanComplete(const DeviceInfo &info);

private:
    QString m_ip;
    bool pingIp(qint64 &responseTime);
    QString getMacAddress(const QString &ip);
    QString resolveHostname(const QString &ip);
    QString getVendorFromMac(const QString &mac);
    QString getNicVendor(const QString &mac);
    QString parseMacFromArp(const QString &ip);
    QString getMacViaArping(const QString &ip);
    QString loadOuiDatabase();
    QString lookupVendor(const QString &macPrefix);
    void sendArpRequest(const QString &ip);
    QString getLocalMacForIp(const QString &ip);
    QString getLocalHostnameForIp(const QString &ip);
    QString resolveHostnameViaNslookup(const QString &ip);
};

#endif // IPSCANNERTHREAD_H
