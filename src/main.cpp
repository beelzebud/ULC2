#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("libretro Updater 2");
    app.setOrganizationName("ulc");

    MainWindow w;
    w.show();

    return app.exec();
}
