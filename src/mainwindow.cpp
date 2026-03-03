#include "mainwindow.h"
#include "ipscanner.h"
#include "ipscannerthread.h"
#include "scannermodel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>
#include <QNetworkInterface>
#include <QNetworkAddressEntry>
#include <QHostAddress>
#include <QSplitter>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QToolBar>
#include <QApplication>
#include <QSettings>
#include <QDateTime>
#include <QElapsedTimer>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_scanner(nullptr)
    , m_scannerThread(nullptr)
    , m_totalScanned(0)
    , m_availableCount(0)
    , m_unavailableCount(0)
    , m_scanStartTime(0)
    , m_isScanning(false)
{
    setupUI();
    initializeScanner();
    populateInterfaces();

    // Update statistics timer
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &MainWindow::updateStatistics);
    m_updateTimer->start(500);

    setMinimumSize(800, 600);
    resize(1024, 768);
}

MainWindow::~MainWindow()
{
    if (m_isScanning && m_scanner) {
        m_scanner->stop();
    }
}

void MainWindow::setupUI()
{
    setWindowTitle("网络扫描器 v2.0");

    // Central widget
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Create toolbar
    QToolBar *toolBar = addToolBar("工具栏");
    toolBar->setIconSize(QSize(24, 24));

    QAction *startAction = new QAction("开始扫描", this);
    connect(startAction, &QAction::triggered, this, &MainWindow::onQuickScan);
    toolBar->addAction(startAction);

    QAction *stopAction = new QAction("停止扫描", this);
    connect(stopAction, &QAction::triggered, this, &MainWindow::onStopScan);
    toolBar->addAction(stopAction);

    toolBar->addSeparator();

    QAction *exportAction = new QAction("导出结果", this);
    connect(exportAction, &QAction::triggered, this, &MainWindow::onExportResults);
    toolBar->addAction(exportAction);

    QAction *clearAction = new QAction("清除结果", this);
    connect(clearAction, &QAction::triggered, this, &MainWindow::onClearResults);
    toolBar->addAction(clearAction);

    toolBar->addSeparator();

    QAction *refreshAction = new QAction("刷新网络接口", this);
    connect(refreshAction, &QAction::triggered, this, &MainWindow::onRefreshInterfaces);
    toolBar->addAction(refreshAction);

    toolBar->addSeparator();

    QAction *aboutAction = new QAction("关于", this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
    toolBar->addAction(aboutAction);

    // Settings group
    m_settingsGroup = new QGroupBox("扫描设置", this);
    QGridLayout *settingsLayout = new QGridLayout(m_settingsGroup);

    settingsLayout->addWidget(new QLabel("网络接口:"), 0, 0);
    m_interfaceCombo = new QComboBox(this);
    connect(m_interfaceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onInterfaceChanged);
    settingsLayout->addWidget(m_interfaceCombo, 0, 1, 1, 2);

    settingsLayout->addWidget(new QLabel("网络地址:"), 1, 0);
    m_networkAddrEdit = new QLineEdit(this);
    m_networkAddrEdit->setPlaceholderText("例如: 192.168.1.0");
    settingsLayout->addWidget(m_networkAddrEdit, 1, 1);

    settingsLayout->addWidget(new QLabel("子网掩码:"), 1, 2);
    m_netmaskEdit = new QLineEdit(this);
    m_netmaskEdit->setPlaceholderText("例如: 255.255.255.0");
    settingsLayout->addWidget(m_netmaskEdit, 1, 3);

    QHBoxLayout *rangeLayout = new QHBoxLayout();
    rangeLayout->addWidget(new QLabel("起始IP:"));
    m_startIpSpinBox = new QSpinBox(this);
    m_startIpSpinBox->setRange(1, 254);
    m_startIpSpinBox->setValue(1);
    rangeLayout->addWidget(m_startIpSpinBox);
    rangeLayout->addWidget(new QLabel("结束IP:"));
    m_endIpSpinBox = new QSpinBox(this);
    m_endIpSpinBox->setRange(1, 254);
    m_endIpSpinBox->setValue(254);
    rangeLayout->addWidget(m_endIpSpinBox);
    rangeLayout->addStretch();

    settingsLayout->addLayout(rangeLayout, 2, 0, 1, 4);

    mainLayout->addWidget(m_settingsGroup);

    // Scan buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_quickScanBtn = new QPushButton("快速扫描整个网段", this));
    m_quickScanBtn->setStyleSheet(
        "QPushButton { padding: 8px 16px; font-size: 14px; font-weight: bold; }"
        "QPushButton:hover { background-color: #4CAF50; color: white; }"
    );
    connect(m_quickScanBtn, &QPushButton::clicked, this, &MainWindow::onQuickScan);

    buttonLayout->addWidget(m_customScanBtn = new QPushButton("自定义范围扫描", this));
    connect(m_customScanBtn, &QPushButton::clicked, this, &MainWindow::onStartScan);

    buttonLayout->addStretch();

    buttonLayout->addWidget(m_stopBtn = new QPushButton("停止扫描", this));
    m_stopBtn->setEnabled(false);
    m_stopBtn->setStyleSheet(
        "QPushButton { padding: 8px 16px; font-size: 14px; font-weight: bold; }"
        "QPushButton:hover { background-color: #f44336; color: white; }"
    );
    connect(m_stopBtn, &QPushButton::clicked, this, &MainWindow::onStopScan);

    buttonLayout->addWidget(m_clearBtn = new QPushButton("清除结果", this));
    connect(m_clearBtn, &QPushButton::clicked, this, &MainWindow::onClearResults);

    buttonLayout->addWidget(m_exportBtn = new QPushButton("导出结果", this));
    connect(m_exportBtn, &QPushButton::clicked, this, &MainWindow::onExportResults);

    mainLayout->addLayout(buttonLayout);

    // Progress bar
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    mainLayout->addWidget(m_progressBar);

    // Results table
    m_resultsTable = new QTableWidget(this);
    m_resultsTable->setColumnCount(6);
    m_resultsTable->setHorizontalHeaderLabels({"IP地址", "状态", "响应时间", "MAC地址", "厂商", "网卡厂商"});
    m_resultsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_resultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_resultsTable->setSortingEnabled(true);
    m_resultsTable->horizontalHeader()->setStretchLastSection(true);
    m_resultsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_resultsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_resultsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_resultsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_resultsTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);

    // Set alternating row colors
    m_resultsTable->setAlternatingRowColors(true);
    m_resultsTable->setStyleSheet(
        "QTableWidget {"
        "  background-color: white;"
        "  gridline-color: #e0e0e0;"
        "}"
        "QTableWidget::item {"
        "  padding: 4px;"
        "}"
        "QHeaderView::section {"
        "  background-color: #2196F3;"
        "  color: white;"
        "  padding: 6px;"
        "  border: none;"
        "  font-weight: bold;"
        "}"
    );

    mainLayout->addWidget(m_resultsTable, 1);

    // Status bar
    createStatusBar();
}

void MainWindow::createStatusBar()
{
    QStatusBar *statusBar = new QStatusBar(this);
    setStatusBar(statusBar);

    m_statusLabel = new QLabel("就绪", this);
    statusBar->addWidget(m_statusLabel);

    m_statsLabel = new QLabel("已扫描: 0 | 空闲: 0 | 占用: 0", this);
    statusBar->addPermanentWidget(m_statsLabel);
}

void MainWindow::initializeScanner()
{
    m_scanner = new IPScanner(this);
    connect(m_scanner, &IPScanner::scanFinished, this, &MainWindow::onScanFinished);
    connect(m_scanner, &IPScanner::scanProgress, this, &MainWindow::onScanProgress);
    connect(m_scanner, &IPScanner::scanResult, this, &MainWindow::onScanResult);
}

void MainWindow::populateInterfaces()
{
    m_interfaceCombo->clear();

    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &interface : interfaces) {
        if (interface.flags() & QNetworkInterface::IsUp &&
            interface.flags() & QNetworkInterface::IsRunning &&
            !(interface.flags() & QNetworkInterface::IsLoopBack)) {

            QList<QNetworkAddressEntry> addressEntries = interface.addressEntries();
            for (const QNetworkAddressEntry &entry : addressEntries) {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    QString itemText = QString("%1 - %2")
                        .arg(interface.humanReadableName())
                        .arg(entry.ip().toString());
                    m_interfaceCombo->addItem(itemText, entry.ip().toString());
                }
            }
        }
    }

    if (m_interfaceCombo->count() > 0) {
        m_interfaceCombo->setCurrentIndex(0);
        onInterfaceChanged();
    }
}

void MainWindow::onInterfaceChanged()
{
    QString currentIp = m_interfaceCombo->currentData().toString();
    if (currentIp.isEmpty()) return;

    // Find the network interface and its settings
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &interface : interfaces) {
        QList<QNetworkAddressEntry> entries = interface.addressEntries();
        for (const QNetworkAddressEntry &entry : entries) {
            if (entry.ip().toString() == currentIp) {
                QString netmask = entry.netmask().toString();
                QString networkAddr = calculateNetworkRange(currentIp, netmask);

                m_networkAddrEdit->setText(networkAddr);
                m_netmaskEdit->setText(netmask);

                break;
            }
        }
    }
}

QString MainWindow::calculateNetworkRange(const QString &ip, const QString &netmask)
{
    QStringList ipParts = ip.split('.');
    QStringList maskParts = netmask.split('.');

    QStringList networkParts;
    for (int i = 0; i < 4; ++i) {
        int ipPart = ipParts[i].toInt();
        int maskPart = maskParts[i].toInt();
        networkParts << QString::number(ipPart & maskPart);
    }

    return networkParts.join('.');
}

void MainWindow::onStartScan()
{
    QString networkAddr = m_networkAddrEdit->text();
    QString netmask = m_netmaskEdit->text();
    int startIp = m_startIpSpinBox->value();
    int endIp = m_endIpSpinBox->value();

    if (networkAddr.isEmpty() || netmask.isEmpty()) {
        QMessageBox::warning(this, "错误", "请设置网络地址和子网掩码！");
        return;
    }

    // Clear previous results
    onClearResults();

    m_isScanning = true;
    updateStartButtonState();

    // Extract base IP (first 3 octets)
    QStringList baseParts = networkAddr.split('.');
    if (baseParts.size() != 4) {
        QMessageBox::warning(this, "错误", "无效的网络地址格式！");
        m_isScanning = false;
        updateStartButtonState();
        return;
    }

    QString baseIp = QString("%1.%2.%3").arg(baseParts[0]).arg(baseParts[1]).arg(baseParts[2]);

    m_statusLabel->setText(QString("正在扫描 %1.%2-%3 ...")
        .arg(baseIp).arg(startIp).arg(endIp));
    m_progressBar->setValue(0);

    m_totalScanned = 0;
    m_availableCount = 0;
    m_unavailableCount = 0;
    m_scanStartTime = QDateTime::currentMSecsSinceEpoch();

    m_scanner->startScan(baseIp, startIp, endIp);
}

void MainWindow::onQuickScan()
{
    QString networkAddr = m_networkAddrEdit->text();
    if (networkAddr.isEmpty()) {
        QMessageBox::warning(this, "错误", "请先选择网络接口或设置网络地址！");
        return;
    }

    // Clear previous results
    onClearResults();

    m_isScanning = true;
    updateStartButtonState();

    QStringList baseParts = networkAddr.split('.');
    QString baseIp = QString("%1.%2.%3").arg(baseParts[0]).arg(baseParts[1]).arg(baseParts[2]);

    m_statusLabel->setText(QString("正在快速扫描整个网段 %1.1-254 ...").arg(baseIp));
    m_progressBar->setValue(0);

    m_totalScanned = 0;
    m_availableCount = 0;
    m_unavailableCount = 0;
    m_scanStartTime = QDateTime::currentMSecsSinceEpoch();

    m_scanner->startScan(baseIp, 1, 254);
}

void MainWindow::onStopScan()
{
    if (m_isScanning && m_scanner) {
        m_scanner->stop();
        m_statusLabel->setText("扫描已停止");
    }
}

void MainWindow::onScanFinished()
{
    m_isScanning = false;
    updateStartButtonState();
    m_progressBar->setValue(100);

    qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - m_scanStartTime;
    m_statusLabel->setText(QString("扫描完成！耗时: %1 秒").arg(elapsed / 1000.0, 0, 'f', 2));
}

void MainWindow::onScanProgress(int current, int total)
{
    int progress = total > 0 ? (current * 100 / total) : 0;
    m_progressBar->setValue(progress);
    m_statusLabel->setText(QString("正在扫描: %1/%2").arg(current).arg(total));
}

void MainWindow::onScanResult(const DeviceInfo &info)
{
    m_totalScanned++;
    if (info.isAvailable) {
        m_unavailableCount++;
    } else {
        m_availableCount++;
    }

    addResultToTable(info);
}

void MainWindow::addResultToTable(const DeviceInfo &info)
{
    int row = m_resultsTable->rowCount();
    m_resultsTable->insertRow(row);

    // IP Address
    QTableWidgetItem *ipItem = new QTableWidgetItem(info.ip);
    ipItem->setTextAlignment(Qt::AlignCenter);
    m_resultsTable->setItem(row, 0, ipItem);

    // Status
    QTableWidgetItem *statusItem = new QTableWidgetItem(info.isAvailable ? "已占用" : "空闲");
    statusItem->setTextAlignment(Qt::AlignCenter);
    m_resultsTable->setItem(row, 1, statusItem);

    // Response Time
    QString timeText = info.isAvailable ? QString("%1 ms").arg(info.responseTime) : "-";
    QTableWidgetItem *timeItem = new QTableWidgetItem(timeText);
    timeItem->setTextAlignment(Qt::AlignCenter);
    m_resultsTable->setItem(row, 2, timeItem);

    // MAC Address
    QTableWidgetItem *macItem = new QTableWidgetItem(info.macAddress);
    macItem->setTextAlignment(Qt::AlignCenter);
    m_resultsTable->setItem(row, 3, macItem);

    // Vendor
    QTableWidgetItem *vendorItem = new QTableWidgetItem(info.vendor);
    vendorItem->setTextAlignment(Qt::AlignCenter);
    m_resultsTable->setItem(row, 4, vendorItem);

    // NIC Vendor
    QTableWidgetItem *nicVendorItem = new QTableWidgetItem(info.nicVendor);
    nicVendorItem->setTextAlignment(Qt::AlignCenter);
    m_resultsTable->setItem(row, 5, nicVendorItem);

    highlightRow(row, info.isAvailable);

    // Auto-scroll to bottom
    m_resultsTable->scrollToBottom();
}

void MainWindow::highlightRow(int row, bool isAvailable)
{
    QColor bgColor = isAvailable ? QColor(255, 235, 238) : QColor(232, 245, 233);
    QColor fgColor = isAvailable ? QColor(183, 28, 28) : QColor(27, 94, 32);

    for (int col = 0; col < m_resultsTable->columnCount(); ++col) {
        QTableWidgetItem *item = m_resultsTable->item(row, col);
        if (item) {
            item->setBackground(bgColor);
            if (col == 1) { // Status column
                item->setForeground(QBrush(fgColor));
                QFont font = item->font();
                font.setBold(true);
                item->setFont(font);
            }
        }
    }
}

void MainWindow::updateStartButtonState()
{
    m_quickScanBtn->setEnabled(!m_isScanning);
    m_customScanBtn->setEnabled(!m_isScanning);
    m_stopBtn->setEnabled(m_isScanning);
    m_interfaceCombo->setEnabled(!m_isScanning);
    m_networkAddrEdit->setEnabled(!m_isScanning);
    m_netmaskEdit->setEnabled(!m_isScanning);
    m_startIpSpinBox->setEnabled(!m_isScanning);
    m_endIpSpinBox->setEnabled(!m_isScanning);
}

void MainWindow::updateStatistics()
{
    m_statsLabel->setText(QString("已扫描: %1 | 空闲: %2 | 占用: %3")
        .arg(m_totalScanned).arg(m_availableCount).arg(m_unavailableCount));
}

void MainWindow::onExportResults()
{
    if (m_resultsTable->rowCount() == 0) {
        QMessageBox::information(this, "提示", "没有可导出的结果！");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(
        this,
        "导出扫描结果",
        QString("network-scan-%1.csv").arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss")),
        "CSV 文件 (*.csv);;文本文件 (*.txt)"
    );

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法打开文件进行写入！");
        return;
    }

    QTextStream out(&file);

    // Write header
    out << "IP地址,状态,响应时间,扫描时间\n";

    // Write data
    for (int row = 0; row < m_resultsTable->rowCount(); ++row) {
        QStringList rowText;
        for (int col = 0; col < m_resultsTable->columnCount(); ++col) {
            rowText << m_resultsTable->item(row, col)->text();
        }
        out << rowText.join(",") << "\n";
    }

    file.close();

    QMessageBox::information(this, "成功", QString("结果已导出到:\n%1").arg(fileName));
}

void MainWindow::onRefreshInterfaces()
{
    populateInterfaces();
    QMessageBox::information(this, "刷新完成", "网络接口列表已刷新！");
}

void MainWindow::onClearResults()
{
    m_resultsTable->setRowCount(0);
    m_totalScanned = 0;
    m_availableCount = 0;
    m_unavailableCount = 0;
    m_progressBar->setValue(0);
    m_statusLabel->setText("就绪");
    updateStatistics();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_isScanning && m_scanner) {
        auto reply = QMessageBox::question(
            this,
            "确认退出",
            "扫描正在进行中，确定要退出吗？",
            QMessageBox::Yes | QMessageBox::No
        );

        if (reply == QMessageBox::Yes) {
            m_scanner->stop();
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        event->accept();
    }
}

void MainWindow::onAbout()
{
    QString aboutText = QString(
        "<h2>网络扫描器 v2.0</h2>"
        "<p><b>功能特性：</b></p>"
        "<ul>"
        "<li>快速扫描局域网设备</li>"
        "<li>获取设备MAC地址</li>"
        "<li>识别网卡厂商</li>"
        "<li>测量响应时间</li>"
        "<li>导出扫描结果</li>"
        "</ul>"
        "<p><b>技术实现：</b></p>"
        "<ul>"
        "<li>基于 Qt6 开发</li>"
        "<li>使用 ARP 协议获取 MAC 地址</li>"
        "<li>内置 OUI 厂商数据库</li>"
        "<li>多线程并发扫描</li>"
        "</ul>"
        "<p><b>版权信息：</b></p>"
        "<p>&copy; 2026 克亮</p>"
        "<p>基于 CMake + Qt6 构建</p>"
    );

    QMessageBox::about(this, "关于网络扫描器", aboutText);
}
