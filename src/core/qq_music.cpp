//
// QQMusic - 使用 Python API Bridge 的 QQ 音乐支持
// Created by AI Assistant on 2026/05/01
//

#include "core/qq_music.h"
#include <QCoreApplication>
#include <QJsonParseError>
#include <QRegularExpression>
#include <QFileInfo>
#include <algorithm>
#include <cmath>

QQMusic::QQMusic(QWidget *parent)
    : BaseMusic(parent)
    , pythonProcess_(nullptr)
{
}

QQMusic::~QQMusic()
{
    stopPythonProcess();
}

bool QQMusic::checkMusicLink(QString musicLink)
{
    if (!musicLink.contains("y.qq.com")) {
        emit errorOccurred("歌单链接格式异常！");
        return false;
    }
    id_ = getNumberFromStr(musicLink.mid(musicLink.indexOf("playlist/") + 9));
    return true;
}

void QQMusic::importMusic()
{
    curIdx_ = 0;
    songsNum_ = 0;
    songs_.clear();
    currSongIndex_ = 0;
    songsList_.clear();
    emit songsNumberChanged(1, 0);
    
    // 启动 Python 进程
    if (!startPythonProcess()) {
        return;
    }
    
    // 直接请求歌单
    QJsonObject params;
    params["id"] = id_;
    sendCommand("songlist_detail", params);
}

void QQMusic::sendCommand(const QString &cmd, const QJsonObject &params)
{
    if (!pythonProcess_ || pythonProcess_->state() == QProcess::NotRunning) {
        return;
    }

    // 构建命令 JSON
    QJsonObject request;
    request["cmd"] = cmd;
    request["params"] = params;

    QByteArray data = QJsonDocument(request).toJson(QJsonDocument::Compact);
    data.append('\n');
    pythonProcess_->write(data);
    pendingCommand_ = request;
}

bool QQMusic::startPythonProcess()
{
    if (pythonProcess_ && pythonProcess_->state() != QProcess::NotRunning) {
        return true;
    }
    
    QStringList pythonExeCandidates;
    QString pythonExe;
    pythonExeCandidates << QCoreApplication::applicationDirPath() + "/../../../d3party/python-3.12.2/python.exe"
              << QCoreApplication::applicationDirPath() + "/python-3.12.2/python.exe";
    for (int i = 0; i < pythonExeCandidates.size(); ++i) {
        QFileInfo file(pythonExeCandidates[i]);
        if (file.isFile()) {
            pythonExe = pythonExeCandidates[i];
            break;
        }
    }

    QString pythonFile;
    QStringList pythonFileCandidates;
    pythonFileCandidates << QCoreApplication::applicationDirPath() + "/../../../d3party/QQMusicApi/qqmusic_bridge.py"
                         << QCoreApplication::applicationDirPath() + "/QQMusicApi/qqmusic_bridge.py";
    for (int i = 0; i < pythonFileCandidates.size(); ++i) {
        QFileInfo file(pythonFileCandidates[i]);
        if (file.isFile()) {
            pythonFile = pythonFileCandidates[i];
            break;
        }
    }

    // 创建进程
    delete pythonProcess_;
    pythonProcess_ = new QProcess(this);
    
    connect(pythonProcess_, &QProcess::readyReadStandardOutput, this, &QQMusic::onProcessReadyRead);
    connect(pythonProcess_, &QProcess::finished, this, &QQMusic::onProcessFinished);
    connect(pythonProcess_, &QProcess::errorOccurred, this, &QQMusic::onProcessErrorOccurred);
    
    // 设置工作目录
    QFileInfo bridgeInfo(pythonFile);
    pythonProcess_->setWorkingDirectory(bridgeInfo.absolutePath());
    
    // 启动 Python 进程
    pythonProcess_->start(pythonExe, QStringList() << pythonFile);
    
    if (!pythonProcess_->waitForStarted(5000)) {
        emit errorOccurred("启动 Python 进程失败！");
        delete pythonProcess_;
        pythonProcess_ = nullptr;
        return false;
    }
    
    return true;
}

void QQMusic::stopPythonProcess()
{
    if (pythonProcess_) {
        pythonProcess_->kill();
        pythonProcess_->waitForFinished(1000);
        delete pythonProcess_;
        pythonProcess_ = nullptr;
    }
}

void QQMusic::onProcessReadyRead()
{
    if (!pythonProcess_) return;
    QByteArray response = pythonProcess_->readAllStandardOutput();
    QJsonDocument doc = QJsonDocument::fromJson(response);
    QString cmd = pendingCommand_["cmd"].toString();
    if (cmd == "songlist_detail") {
        QJsonArray songs = doc["data"].toObject()["songs"].toArray();
        songsNum_ = songs.size();
        songsList_.resize(songsNum_);
        songs_.resize(songsNum_);
        for (int i = 0; i < songs.size(); ++i) {
            QJsonObject params;
            QJsonObject song = songs[i].toObject();
            QString song_name = song["name"].toString();
            QJsonArray singers = song["singer"].toArray();
            QString singer_name;
            for(int i = 0; i < singers.size(); ++i) {
                singer_name += singers[i].toObject()["name"].toString();
                if (i != singers.size() - 1)
                    singer_name += ",";
            }
            params["id"] = song["id"].toInt();
            sendCommand("lyric", params);

            songs_[i].lyric.emplace_back(song_name);
            songs_[i].lyric.emplace_back(singer_name);
            songs_[i].time.emplace_back(INT32_MIN);
            songs_[i].time.emplace_back(INT32_MIN);
            songs_[i].tLyric.emplace_back("[]");
            songs_[i].tLyric.emplace_back("[]");
            songs_[i].rLyric.emplace_back("[]");
            songs_[i].rLyric.emplace_back("[]");
        }
    } else if (cmd == "lyric") {
        QString lyric = doc["data"].toObject()["lyric"].toString();
        QString trans = doc["data"].toObject()["trans"].toString();

        QStringList lines = lyric.split('\n', Qt::SkipEmptyParts);
        std::vector<std::pair<int, QString>> unsyncLyric;
        for (const QString& line : lines) {
            QString content = line;
            content.remove(tRe_);
            content.trimmed();
            if (content.isEmpty() || content == "//")
                continue;
            for (auto it = symbolMap.constBegin(); it != symbolMap.constEnd(); ++it) {
                content.replace(it.key(), it.value());
            }

            QRegularExpressionMatchIterator timeIt = tRe_.globalMatch(line);
            while (timeIt.hasNext()) {
                QRegularExpressionMatch timeMatch = timeIt.next();
                int ms = timeToMilliseconds(timeMatch.captured(1));
                if (ms != -1) {
                    unsyncLyric.emplace_back(std::pair<int, QString>(ms, content));
                }
            }
        }
        std::sort(unsyncLyric.begin(), unsyncLyric.end(), [](const auto &p1, const auto &p2){
            return p1.first < p2.first;
        });
        for (const auto &lrc : unsyncLyric) {
            songs_[curIdx_].lyric.emplace_back(lrc.second);
            songs_[curIdx_].time.emplace_back(lrc.first);
            songs_[curIdx_].tLyric.emplace_back("[]");
            songs_[curIdx_].rLyric.emplace_back("[]");
        }
        QTime time(0, 0, 0, 0);
        time = time.addMSecs(unsyncLyric.back().first);
        QString duration = time.toString("mm:ss");

        lines = trans.split('\n', Qt::SkipEmptyParts);
        for (const QString& line : lines) {
            QString content = line;
            content.remove(tRe_);
            content.trimmed();
            if (content.isEmpty() || content == "//")
                continue;
            for (auto it = symbolMap.constBegin(); it != symbolMap.constEnd(); ++it) {
                content.replace(it.key(), it.value());
            }

            QRegularExpressionMatchIterator timeIt = tRe_.globalMatch(line);
            while (timeIt.hasNext()) {
                QRegularExpressionMatch timeMatch = timeIt.next();
                int ms = timeToMilliseconds(timeMatch.captured(1));
                if (ms != -1) {
                    for (int i = 0, j = songs_[curIdx_].lyric.size(); i < j; ++i) {
                        int delta = std::abs(ms - songs_[curIdx_].time[i]);
                        if (delta <= 1) {
                            songs_[curIdx_].tLyric[i] = content;
                        }
                    }
                }
            }
        }

        QStringList songList;
        songList << QString::number(curIdx_ + 1) << songs_[curIdx_].lyric[0] << songs_[curIdx_].lyric[1] << duration << (trans.isEmpty() ? "⛔️" : "❌") << "⛔️";
        songsList_[curIdx_] = songList;

        int progress = songsNum_;
        {
            std::lock_guard<std::mutex> lock(mtx);
            for (const QStringList& str : songsList_) {
                if (str.isEmpty())
                    progress--;
            }
            emit songsNumberChanged(songsNum_, progress);
        }

        curIdx_++;
        emit songsNumberChanged(songsNum_, curIdx_);
    } else {

    }

    if (curIdx_ == songsNum_) {
        emit taskFinished();
    }
}

void QQMusic::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);

    if (exitStatus == QProcess::CrashExit) {
        emit errorOccurred("Python 进程意外崩溃！");
    }

    if (pythonProcess_) {
        pythonProcess_->deleteLater();
        pythonProcess_ = nullptr;
    }
}

void QQMusic::onProcessErrorOccurred(QProcess::ProcessError error)
{
    QString errorMsg;
    switch (error) {
        case QProcess::FailedToStart:
            errorMsg = "启动 Python 进程失败！";
            break;
        case QProcess::Crashed:
            errorMsg = "Python 进程已崩溃！";
            break;
        case QProcess::Timedout:
            errorMsg = "等待 Python 进程启动超时！";
            break;
        default:
            errorMsg = "Python 进程发生未知错误！";
            break;
    }
    emit errorOccurred(errorMsg);
}
