#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("Image Viewer");
    a.setApplicationDisplayName("Image Viewer");
    //a.setWindowIcon(QIcon(":images/images/image viewer icon.png"));
    MainWindow w;
    w.showMaximized();
  //  w.show();
    return a.exec();
}
