#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QStyleFactory>
#include <QFont>
#include <QDebug>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set application metadata
    app.setApplicationName("NetworkScanner");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("NetTool");

    // Set application style
    app.setStyle(QStyleFactory::create("Fusion"));

    // Set dark/light theme palette
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(240, 240, 240));
    palette.setColor(QPalette::WindowText, QColor(0, 0, 0));
    palette.setColor(QPalette::Base, QColor(255, 255, 255));
    palette.setColor(QPalette::AlternateBase, QColor(245, 245, 245));
    palette.setColor(QPalette::ToolTipBase, QColor(255, 255, 220));
    palette.setColor(QPalette::ToolTipText, QColor(0, 0, 0));
    palette.setColor(QPalette::Text, QColor(0, 0, 0));
    palette.setColor(QPalette::Button, QColor(240, 240, 240));
    palette.setColor(QPalette::ButtonText, QColor(0, 0, 0));
    palette.setColor(QPalette::BrightText, QColor(255, 0, 0));
    palette.setColor(QPalette::Highlight, QColor(33, 150, 243));
    palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    app.setPalette(palette);

    // Set default font
    QFont font = app.font();
    font.setFamily("Noto Sans");
    font.setPointSize(10);
    app.setFont(font);

    // Create and show main window
    MainWindow window;
    window.show();

    return app.exec();
}
