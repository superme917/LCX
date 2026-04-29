#include "main_window.h"
#include "music_window.h"

#include "ElaContentDialog.h"
#include "ElaEventBus.h"
#include "ElaTheme.h"
#include <QApplication>
#include <QDebug>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QSettings>
#ifdef Q_OS_WIN
#include "ElaApplication.h"
#include <QTimer>
#endif
MainWindow::MainWindow(QWidget* parent)
    : ElaWindow(parent)
{
    initWindow();

    //额外布局
    initEdgeLayout();

    //中心窗口
    initContent();

    // 设置用户偏好
    initSetting();

    // 拦截默认关闭事件
    close_dialog_ = new ElaContentDialog(this);
    close_dialog_->setLeftButtonText("取消");
    close_dialog_->setMiddleButtonText("最小化");
    close_dialog_->setRightButtonText("确定");
    connect(close_dialog_, &ElaContentDialog::rightButtonClicked, this, &MainWindow::closeWindow);
    connect(close_dialog_, &ElaContentDialog::middleButtonClicked, this, [=]() {
        close_dialog_->close();
        showMinimized();
    });
    this->setIsDefaultClosed(false);
    connect(this, &MainWindow::closeButtonClicked, this, [=]() {
        close_dialog_->exec();
    });

    //移动到中心
    moveToCenter();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initWindow()
{
    setFocusPolicy(Qt::StrongFocus);
    resize(1200, 740);
    setUserInfoCardPixmap(QPixmap(":/resource/liu.jpg"));
    setUserInfoCardTitle("子非鱼");
    setUserInfoCardSubTitle("💐🌸🌷🍀🌹🌻🌺");
}

void MainWindow::initEdgeLayout()
{
}

void MainWindow::initContent()
{
    music_window_ = new MusicWindow(this);
    addPageNode("HOME", music_window_, ElaIconType::Music);
    qDebug() << "已注册的事件列表" << ElaEventBus::getInstance()->getRegisteredEventsName();
}

void MainWindow::initSetting()
{
    QSettings setting(QApplication::applicationDirPath() + "/settings.ini", QSettings::Format::IniFormat);
    eTheme->setThemeMode(setting.value("Theme", "Light").toString() == "Light" ? ElaThemeType::ThemeMode::Light : ElaThemeType::ThemeMode::Dark);
    QString DisplayModeArea = setting.value("DisplayModeArea", "Auto").toString();
    setNavigationBarDisplayMode(DisplayModeArea == "Auto" ? ElaNavigationType::NavigationDisplayMode::Auto :
                                    DisplayModeArea == "Compact" ? ElaNavigationType::NavigationDisplayMode::Compact :
                                    DisplayModeArea == "Maximal" ? ElaNavigationType::NavigationDisplayMode::Maximal :
                                    ElaNavigationType::NavigationDisplayMode::Minimal);
}
