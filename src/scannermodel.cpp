#include "scannermodel.h"

#include <QColor>
#include <QFont>

ScannerModel::ScannerModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int ScannerModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_results.size();
}

int ScannerModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return COLUMN_COUNT;
}

QVariant ScannerModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_results.size()) {
        return QVariant();
    }

    const ScanResult &result = m_results.at(index.row());

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case IP_COLUMN:
            return result.ip;
        case STATUS_COLUMN:
            return result.isAvailable ? "已占用" : "空闲";
        case RESPONSE_TIME_COLUMN:
            return result.isAvailable ? QString("%1 ms").arg(result.responseTime) : "-";
        case SCAN_TIME_COLUMN:
            return result.scanTime;
        }
    } else if (role == Qt::DecorationRole && index.column() == STATUS_COLUMN) {
        // Return icon based on status
        return QVariant();
    } else if (role == Qt::ForegroundRole && index.column() == STATUS_COLUMN) {
        return result.isAvailable ? QColor(183, 28, 28) : QColor(27, 94, 32);
    } else if (role == Qt::BackgroundRole) {
        return result.isAvailable ? QColor(255, 235, 238) : QColor(232, 245, 233);
    } else if (role == Qt::FontRole && index.column() == STATUS_COLUMN) {
        QFont font;
        font.setBold(true);
        return font;
    } else if (role == Qt::TextAlignmentRole) {
        return Qt::AlignCenter;
    }

    return QVariant();
}

QVariant ScannerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case IP_COLUMN:
            return "IP地址";
        case STATUS_COLUMN:
            return "状态";
        case RESPONSE_TIME_COLUMN:
            return "响应时间";
        case SCAN_TIME_COLUMN:
            return "扫描时间";
        }
    }

    return QVariant();
}

Qt::ItemFlags ScannerModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return QAbstractTableModel::flags(index);
}

void ScannerModel::addResult(const ScanResult &result)
{
    beginInsertRows(QModelIndex(), m_results.size(), m_results.size());
    m_results.append(result);
    endInsertRows();
}

void ScannerModel::clear()
{
    beginResetModel();
    m_results.clear();
    endResetModel();
}

int ScannerModel::availableCount() const
{
    int count = 0;
    for (const ScanResult &result : m_results) {
        if (!result.isAvailable) {
            count++;
        }
    }
    return count;
}

int ScannerModel::unavailableCount() const
{
    int count = 0;
    for (const ScanResult &result : m_results) {
        if (result.isAvailable) {
            count++;
        }
    }
    return count;
}
