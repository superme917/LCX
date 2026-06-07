#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "core/music_interface/base_music_interface.h"

#include <QLabel>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class QTimer;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnImport_clicked();                                         // 歌单导入
    void onErrorOccured(const QString &msg);                             // 各种错误
    void OnMusicInterfaceTaskFinished();                                 // 音乐导入完成
    void OnMusicInterfaceSongsNumberChanged(int totalNum, int currNum);  // 音乐导入进度

private:
    // 网络检测相关函数
    void startNetworkTimer();
    void checkNetworkStatus();
    void updateNetworkStatus(bool connected);

    // 启动后台服务器
    void startServer();

    // 端口检测
    void checkPort(const QString &host, quint16 port, QLabel *label, bool *start);

    Ui::MainWindow *ui_ = nullptr;
    QTimer *networkTimer_ = nullptr;  // 定时检测网络
    bool networkConnected_ = false;   // 当前网络状态
    BaseMusicInterface *music_interface_ = nullptr;
    bool qq_music_server_start_ = false;
};

#endif  // MAINWINDOW_H
