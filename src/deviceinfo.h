#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QString>

struct DeviceInfo {
    QString ip;
    bool isAvailable;
    qint64 responseTime;
    QString macAddress;
    QString vendor;
    QString hostname;
    QString nicVendor;

    DeviceInfo()
        : isAvailable(false), responseTime(-1) {}

    DeviceInfo(const QString &ip_, bool available, qint64 time)
        : ip(ip_), isAvailable(available), responseTime(time) {}
};

Q_DECLARE_METATYPE(DeviceInfo)

#endif // DEVICEINFO_H
