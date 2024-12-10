#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow w;
    w.setWindowTitle("英文单词检索");
    w.resize(400,300);
    w.show();

    return app.exec();
}
