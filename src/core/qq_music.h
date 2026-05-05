//
// Create By WangYiFan on 2026/05/01
//

#pragma once

#include "core/base_music.h"

#include <QJsonObject>
#include <QProcess>

namespace LCX::core {

// QQ音乐解析接口
class QQMusic : public BaseMusic {
    Q_OBJECT
public:
    // 构造函数
    QQMusic(QWidget *parent = nullptr);
    // 析构函数
    ~QQMusic();
    // 解析歌单链接，获取歌词
    virtual void importMusic(const QString &playlist_link) override;

private slots:
    void onProcessReadyRead();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessErrorOccurred(QProcess::ProcessError error);

private:
    bool startPythonProcess();
    void stopPythonProcess();
    void sendCommand(const QString &cmd, const QJsonObject &params);

private:
    QProcess *python_process_ = nullptr;
    QJsonObject pending_command_;
};

}  // namespace LCX::core
