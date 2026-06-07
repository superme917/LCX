#include <QApplication>
#include "app/main_window.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("LCX");
    app.setApplicationVersion("0.1");

    MainWindow w;
    w.show();

    return app.exec();
}