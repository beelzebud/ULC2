#include <QApplication>
#include <QFontDatabase>
#include <QObject>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Emu-Manager");
    app.setOrganizationName("Emu-Manager");

    // Embed and register Aldrich so it works without system installation
    const int fontId = QFontDatabase::addApplicationFont(":/fonts/Aldrich-Regular.ttf");
    if (fontId == -1)
        qWarning("Failed to load embedded Aldrich font.");

    MainWindow w;
    w.show();

    return app.exec();
}
