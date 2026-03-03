#ifndef SCANNERMODEL_H
#define SCANNERMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include <QHostAddress>
#include <QDateTime>

struct ScanResult {
    QString ip;
    bool isAvailable;
    qint64 responseTime;
    QString scanTime;

    ScanResult()
        : isAvailable(false), responseTime(-1) {}

    ScanResult(const QString &ip_, bool available, qint64 time)
        : ip(ip_), isAvailable(available), responseTime(time)
        , scanTime(QDateTime::currentDateTime().toString("HH:mm:ss")) {}
};

class ScannerModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        IP_COLUMN = 0,
        STATUS_COLUMN,
        RESPONSE_TIME_COLUMN,
        SCAN_TIME_COLUMN,
        COLUMN_COUNT
    };

    enum Role {
        IpRole = Qt::UserRole + 1,
        StatusRole,
        ResponseTimeRole,
        ScanTimeRole
    };

    explicit ScannerModel(QObject *parent = nullptr);

    // QAbstractTableModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // Custom methods
    void addResult(const ScanResult &result);
    void clear();
    QList<ScanResult> results() const { return m_results; }
    int availableCount() const;
    int unavailableCount() const;

private:
    QList<ScanResult> m_results;
};

#endif // SCANNERMODEL_H
