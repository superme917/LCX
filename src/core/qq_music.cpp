//
// Create By WangYiFan on 2026/05/01
//

#include "core/qq_music.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QRegularExpression>
#include <algorithm>
#include <cmath>

namespace LCX::core {

QQMusic::QQMusic(QWidget *parent) : BaseMusic(parent) {}

// 停止Python进程
QQMusic::~QQMusic() { stopPythonProcess(); }

void QQMusic::importMusic(const QString &playlist_link) {
    // 获取歌单ID
    QRegularExpression id_regular("playlist/(\\d+)");
    QRegularExpressionMatch id_match = id_regular.match(playlist_link);
    QString playlist_id;
    if (id_match.hasMatch()) {
        playlist_id = id_match.captured(1);
    } else {
        emit errorOccurred("歌单链接好像有问题哦，请检查一下😉");
        return;
    }

    // 清空上一次的解析数据
    songs_.clear();
    emit songsNumberChanged(1, 0);

    // 启动 Python 进程
    if (!startPythonProcess()) {
        return;
    }

    // 解析歌单，获取歌单下的歌曲信息
    QJsonObject params;
    params["id"] = playlist_id;
    sendCommand("songlist_detail", params);
}

void QQMusic::sendCommand(const QString &cmd, const QJsonObject &params) {
    if (!python_process_ || python_process_->state() == QProcess::NotRunning) {
        return;
    }

    // 构建命令 JSON
    QJsonObject request;
    request["cmd"] = cmd;
    request["params"] = params;

    QByteArray data = QJsonDocument(request).toJson(QJsonDocument::Compact);
    data.append('\n');
    python_process_->write(data);
    pending_command_ = request;
}

bool QQMusic::startPythonProcess() {
    if (python_process_ && python_process_->state() != QProcess::NotRunning) {
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
    delete python_process_;
    python_process_ = new QProcess(this);

    connect(python_process_, &QProcess::readyReadStandardOutput, this, &QQMusic::onProcessReadyRead);
    connect(python_process_, &QProcess::finished, this, &QQMusic::onProcessFinished);
    connect(python_process_, &QProcess::errorOccurred, this, &QQMusic::onProcessErrorOccurred);

    // 设置工作目录
    QFileInfo bridgeInfo(pythonFile);
    python_process_->setWorkingDirectory(bridgeInfo.absolutePath());

    // 启动 Python 进程
    python_process_->start(pythonExe, QStringList() << pythonFile);

    if (!python_process_->waitForStarted(5000)) {
        emit errorOccurred("启动 Python 进程失败😰，It's time to 寻找外援了。");
        delete python_process_;
        python_process_ = nullptr;
        return false;
    }

    return true;
}

void QQMusic::stopPythonProcess() {
    if (python_process_) {
        python_process_->kill();
        python_process_->waitForFinished(1000);
        delete python_process_;
        python_process_ = nullptr;
    }
}

void QQMusic::onProcessReadyRead() {
    if (!python_process_) return;
    static int current_song_idx = 0;
    QByteArray response = python_process_->readAllStandardOutput();
    QJsonDocument doc = QJsonDocument::fromJson(response);
    QString cmd = pending_command_["cmd"].toString();
    if (cmd == "songlist_detail") {
        QJsonArray songs = doc["data"].toObject()["songs"].toArray();
        songs_.resize(songs.size());
        for (int i = 0; i < songs.size(); ++i) {
            // 获取歌曲信息，并填入songs_
            QJsonObject song = songs[i].toObject();
            QJsonArray singers = song["singer"].toArray();   // 歌手
            QString name = song["name"].toString();          // 歌名
            int song_id = song["id"].toInt();                // 歌曲ID
            int duration = song["interval"].toInt() * 1000;  // 歌曲时长

            QString singer;
            for (int i = 0; i < singers.size(); ++i) {
                singer += singers[i].toObject()["name"].toString();
                if (i != singers.size() - 1) singer += "/";
            }

            QJsonObject params;
            params["id"] = song_id;
            sendCommand("lyric", params);

            songs_[i].singer = singer;
            songs_[i].duration = duration;
            songs_[i].name = name;
        }
        current_song_idx = 0;
    } else if (cmd == "lyric") {
        // 解析原始歌词，填入songs_
        QString lyric = doc["data"].toObject()["lyric"].toString();
        QStringList lines = lyric.split('\n', Qt::SkipEmptyParts);
        std::vector<std::pair<int, QString>> unsynced_lyric;
        for (const QString &line : lines) {
            QString content = line;
            content.remove(time_regular_);
            content = content.trimmed().remove(QRegularExpression("\\s+$"));
            if (content.isEmpty() || content == "//") continue;
            for (auto it = symbolMap.constBegin(); it != symbolMap.constEnd(); ++it) {
                content.replace(it.key(), it.value());
            }

            int ms = timeToMilliseconds(line);
            if (ms != -1) {
                unsynced_lyric.emplace_back(std::pair<int, QString>(ms, content));
            }
        }
        // 按照时间戳对歌词进行排序
        std::sort(unsynced_lyric.begin(), unsynced_lyric.end(),
                  [](const auto &p1, const auto &p2) { return p1.first < p2.first; });

        for (const auto &lrc : unsynced_lyric) {
            songs_[current_song_idx].lyric.emplace_back(lrc.second);
            songs_[current_song_idx].time.emplace_back(lrc.first);
            songs_[current_song_idx].tLyric.emplace_back("");
            songs_[current_song_idx].rLyric.emplace_back("");
        }

        // 解析翻译歌词
        QString trans = doc["data"].toObject()["trans"].toString();
        lines = trans.split('\n', Qt::SkipEmptyParts);
        for (const QString &line : lines) {
            QString content = line;
            content.remove(time_regular_);
            content = content.trimmed().remove(QRegularExpression("\\s+$"));
            if (content.isEmpty() || content == "//") continue;
            for (auto it = symbolMap.constBegin(); it != symbolMap.constEnd(); ++it) {
                content.replace(it.key(), it.value());
            }

            // 通过时间戳对齐原始歌词和翻译歌词
            int ms = timeToMilliseconds(line);
            if (ms != -1) {
                auto it = std::min_element(songs_[current_song_idx].time.begin(), songs_[current_song_idx].time.end(),
                                           [ms](int a, int b) { return std::abs(a - ms) < std::abs(b - ms); });
                if (it != songs_[current_song_idx].time.end()) {
                    int idx = std::distance(songs_[current_song_idx].time.begin(), it);
                    songs_[current_song_idx].tLyric[idx] = content;
                }
            }
            songs_[current_song_idx].has_tLyric = true;
        }

        // 解析音译歌词
        QString roma = doc["data"].toObject()["roma"].toString();
        QRegularExpression content_regular(R"(LyricContent=\"([^\"]+)\")");
        QRegularExpression time_regular(R"(\[(\d+),(\d+)\](.*?)\()");
        QRegularExpression roma_regular(R"(\((\d+),(\d+)\)([^\(]+))");
        QRegularExpressionMatch match = content_regular.match(roma);
        if (match.hasMatch()) {
            QString lyric_content = match.captured(1);
            lines = lyric_content.split('\n', Qt::SkipEmptyParts);
            int ms = INT32_MAX;
            QString content;
            for (const QString &line : lines) {
                content.clear();
                QRegularExpressionMatch first_roma = time_regular.match(line);
                if (first_roma.hasMatch()) {
                    ms = first_roma.captured(1).toInt();
                    QString ro = first_roma.captured(3);
                    if (ro.isEmpty() || ro == " ") {
                        continue;
                    }
                    content += ro;
                    QRegularExpressionMatchIterator second_roma = roma_regular.globalMatch(line);
                    while (second_roma.hasNext()) {
                        content += second_roma.next().captured(3);
                    }
                    content = content.trimmed().remove(QRegularExpression("\\s+$"));
                }

                // 找到时间差最小的索引
                auto it = std::min_element(songs_[current_song_idx].time.begin(), songs_[current_song_idx].time.end(),
                                           [ms](int a, int b) { return std::abs(a - ms) < std::abs(b - ms); });
                if (it != songs_[current_song_idx].time.end()) {
                    int idx = std::distance(songs_[current_song_idx].time.begin(), it);
                    songs_[current_song_idx].rLyric[idx] = content;
                }
            }
            songs_[current_song_idx].has_rLyric = true;
        }

        // 发送歌词解析进度信号
        current_song_idx++;
        emit songsNumberChanged(songs_.size(), current_song_idx);
    }
    // 发送任务完成信号
    if (current_song_idx == songs_.size()) {
        emit taskFinished();
    }
}

void QQMusic::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    Q_UNUSED(exitCode);

    if (exitStatus == QProcess::CrashExit) {
        emit errorOccurred("Python 进程意外崩溃！");
    }

    if (python_process_) {
        python_process_->deleteLater();
        python_process_ = nullptr;
    }
}

void QQMusic::onProcessErrorOccurred(QProcess::ProcessError error) {
    QString errorMsg;
    switch (error) {
        case QProcess::FailedToStart:
            errorMsg = "启动 Python 进程失败😰，It's time to 寻找外援了。";
            break;
        case QProcess::Crashed:
            errorMsg = "Python 进程已崩溃😰，It's time to 寻找外援了。";
            break;
        case QProcess::Timedout:
            errorMsg = "等待 Python 进程启动超时😰，It's time to 寻找外援了。";
            break;
        default:
            errorMsg = "Python 进程发生未知错误😰，It's time to 寻找外援了。";
            break;
    }
    emit errorOccurred(errorMsg);
}

}  // namespace LCX::core