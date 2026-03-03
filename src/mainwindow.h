#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QProgressBar>
#include <QLabel>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QSpinBox>
#include <QStatusBar>
#include <QTimer>
#include "deviceinfo.h"

class IPScanner;
class IPScannerThread;
class ScannerModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onStartScan();
    void onStopScan();
    void onScanFinished();
    void onScanProgress(int current, int total);
    void onScanResult(const DeviceInfo &info);
    void onInterfaceChanged();
    void onExportResults();
    void onRefreshInterfaces();
    void onQuickScan();
    void onClearResults();
    void onAbout();
    void updateStatistics();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void setupUI();
    void createStatusBar();
    void initializeScanner();
    void updateStartButtonState();
    void populateInterfaces();
    QString getNetworkInterface();
    QString getNetworkAddress(const QString &interfaceName);
    QString getNetmask(const QString &interfaceName);
    QString calculateNetworkRange(const QString &networkAddr, const QString &netmask);
    void addResultToTable(const DeviceInfo &info);
    void highlightRow(int row, bool isAvailable);

    // UI Components
    QGroupBox *m_settingsGroup;
    QGroupBox *m_networkGroup;
    QComboBox *m_interfaceCombo;
    QLineEdit *m_networkAddrEdit;
    QLineEdit *m_netmaskEdit;
    QSpinBox *m_startIpSpinBox;
    QSpinBox *m_endIpSpinBox;
    QPushButton *m_quickScanBtn;
    QPushButton *m_customScanBtn;
    QPushButton *m_stopBtn;
    QPushButton *m_exportBtn;
    QPushButton *m_clearBtn;
    QPushButton *m_refreshBtn;
    QTableWidget *m_resultsTable;
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    QLabel *m_statsLabel;
    QTimer *m_updateTimer;

    // Scanner Components
    IPScanner *m_scanner;
    QList<IPScannerThread*> m_scannerThreads;
    QThread *m_scannerThread;

    // Statistics
    int m_totalScanned;
    int m_availableCount;
    int m_unavailableCount;
    qint64 m_scanStartTime;

    bool m_isScanning;
};

#endif // MAINWINDOW_H
