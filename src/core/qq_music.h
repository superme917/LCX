//
// QQMusic - 使用 Python API Bridge 的 QQ 音乐支持
// Created by AI Assistant on 2026/05/01
//

#ifndef QQ_MUSIC_H
#define QQ_MUSIC_H

#include "core/base_music.h"
#include <QProcess>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>

class QQMusic : public BaseMusic
{
    Q_OBJECT
public:
    QQMusic(QWidget *parent = nullptr);
    ~QQMusic();

    virtual bool checkMusicLink(QString musicLink) override;
    virtual void importMusic() override;

private slots:
    void onProcessReadyRead();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessErrorOccurred(QProcess::ProcessError error);

private:
    bool startPythonProcess();
    void stopPythonProcess();
    void sendCommand(const QString &cmd, const QJsonObject &params);

private:
    QProcess *pythonProcess_;
    int curIdx_ = 0;
    QJsonObject pendingCommand_;
};

#endif // QQ_MUSIC_H
