#include <QApplication>
#include "mainwindow.h"
#include "themes.h"    // подключаем тему

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // подключаем тему
    app.setStyleSheet(QString::fromUtf8(DARK_THEME));

    MainWindow window;
    window.show();
    return app.exec();
}
