//
// Create By WangYiFan on 2026/04/30
//

#include <QApplication>
#include <QScreen>

#include <ElaWidgetTools/ElaApplication.h>
#include "main_window.h"
#include <Windows.h>

int main(int argc, char* argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication a(argc, argv);
    eApp->init();
    MainWindow w;
    w.show();
    return a.exec();
}
