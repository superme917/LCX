#include "app/main_window.h"
#include "app/ui_main_window.h"
#include "core/music_interface/qq_music_interface.h"
#include "core/process/server_process.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStringList>
#include <QTcpSocket>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui_(new Ui::MainWindow), networkTimer_(new QTimer(this)) {
    ui_->setupUi(this);

    setWindowIcon(QIcon(":/icons/liu.ico"));

    // 启动定时器，持续检测网络状态
    startNetworkTimer();
    // 启动服务器
    startServer();
}

MainWindow::~MainWindow() { delete ui_; }

void MainWindow::startNetworkTimer() {
    connect(networkTimer_, &QTimer::timeout, this, &MainWindow::checkNetworkStatus);
    networkTimer_->start(5000);  // 5 秒

    // 立即执行首次检测
    checkNetworkStatus();
}

void MainWindow::on_btnImport_clicked() {
    QString playlist_id = ui_->editPlaylistUrl->text();
    if (playlist_id.isEmpty()) {
        QMessageBox::warning(this, "提示", "还没有填入歌单链接呀😴");
    } else {
        // 释放旧的音乐接口
        if (music_interface_) {
            music_interface_->deleteLater();
            music_interface_ = nullptr;
        }

        if (ui_->comboPlatform->currentIndex() == 0) {         // 网易云音乐
        } else if (ui_->comboPlatform->currentIndex() == 1) {  // QQ音乐
            if (!qq_music_server_start_) {
                onErrorOccured("QQ音乐服务器未启动，重启一下软件吧😘");
                return;
            }
            music_interface_ = new QQMusicInterface(this);
        } else if (ui_->comboPlatform->currentIndex() == 2) {  // 酷狗音乐
        } else if (ui_->comboPlatform->currentIndex() == 3) {  // 汽水音乐
        }

        connect(music_interface_, &BaseMusicInterface::errorOccurred, this, &MainWindow::onErrorOccured);
        connect(music_interface_, &BaseMusicInterface::taskFinished, this, &MainWindow::OnMusicInterfaceTaskFinished);
        connect(music_interface_, &BaseMusicInterface::songsNumberChanged, this,
                &MainWindow::OnMusicInterfaceSongsNumberChanged);
        music_interface_->ParsePlaylistId(playlist_id);
        music_interface_->ImportMusic();
    }
}

void MainWindow::onErrorOccured(const QString &msg) { QMessageBox::critical(this, "错误", msg); }

void MainWindow::OnMusicInterfaceTaskFinished() {
    QMessageBox::information(this, "提示", "歌单导入成功，我们可太棒了🎉");
}

void MainWindow::OnMusicInterfaceSongsNumberChanged(int totalNum, int currNum) {}

void MainWindow::checkNetworkStatus() {
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkReply *reply = manager->get(QNetworkRequest(QUrl("https://www.baidu.com")));

    connect(reply, &QNetworkReply::finished, this, [this, reply, manager]() {
        networkConnected_ = (reply->error() == QNetworkReply::NoError);
        updateNetworkStatus(networkConnected_);
        reply->deleteLater();
        manager->deleteLater();
    });
}

void MainWindow::updateNetworkStatus(bool connected) {
    if (connected) {
        ui_->labelNetworkIcon->setText(QStringLiteral("<span style='color: #00cc66; font-size: 18px;'>●</span>"));
        ui_->labelNetworkStatus->setText(QStringLiteral("网络状态：已连接"));
        ui_->labelNetworkStatus->setStyleSheet("color: green;");
    } else {
        ui_->labelNetworkIcon->setText(QStringLiteral("<span style='color: #cc0000; font-size: 18px;'>●</span>"));
        ui_->labelNetworkStatus->setText(QStringLiteral("网络状态：未连接"));
        ui_->labelNetworkStatus->setStyleSheet("color: red;");
    }
}

void MainWindow::startServer() {
    // 启动qq音乐服务器
    QStringList candidate_path;
    QString qq_music_binary;
    candidate_path << QCoreApplication::applicationDirPath() + "/../../../thirdparty/QQMusicApi/QQMusicWeb";
    candidate_path << QCoreApplication::applicationDirPath() + "/../../../thirdparty/QQMusicApi/QQMusicWeb.exe";
    candidate_path << QCoreApplication::applicationDirPath() + "/thirdparty/QQMusicApi/QQMusicWeb";
    candidate_path << QCoreApplication::applicationDirPath() + "/thirdparty/QQMusicApi/QQMusicWeb.exe";
    for (int i = 0; i < candidate_path.size(); ++i) {
        QFileInfo file(candidate_path[i]);
        if (file.exists() && file.isExecutable()) {
            qq_music_binary = candidate_path[i];
            break;
        }
    }
    if (qq_music_binary.isEmpty()) {
        onErrorOccured("未找到QQ音乐服务器😦");
    } else {
        ServerProcess *qq_music_server = new ServerProcess(this);
        qq_music_server->start(qq_music_binary);
        connect(qq_music_server, &ServerProcess::errorOccurred, this, &MainWindow::onErrorOccured);
        checkPort("127.0.0.1", 8080, ui_->labelQQMusicServer, &qq_music_server_start_);
    }
}

void MainWindow::checkPort(const QString &host, quint16 port, QLabel *label, bool *start) {
    auto *socket = new QTcpSocket(this);
    socket->connectToHost(host, port);
    connect(socket, &QTcpSocket::connected, this, [this, socket, label, start]() {
        label->setStyleSheet("color: #00cc66; font-size: 12px; font-weight: bold;");
        *start = true;
        socket->deleteLater();
    });
    connect(socket, &QTcpSocket::errorOccurred, this, [this, socket, label, start](QAbstractSocket::SocketError) {
        label->setStyleSheet("color: #dc3545; font-size: 12px; font-weight: bold;");
        *start = false;
        socket->deleteLater();
    });
}