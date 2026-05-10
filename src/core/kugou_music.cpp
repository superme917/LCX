//
// Create By WangYiFan on 2026/05/10
//

#include "core/kugou_music.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <algorithm>

namespace LCX::core {

KuGouMusic::KuGouMusic(QWidget *parent)
    : BaseMusic(parent), network_manager_(new QNetworkAccessManager(this)), kugou_server_process_(new QProcess(this)) {
    // 启动酷狗 API 服务器
    QStringList exeCandidates;
    exeCandidates << QCoreApplication::applicationDirPath() + "/../../../d3party/KuGouMusicApi/app_win.exe"
                  << QCoreApplication::applicationDirPath() + "/KuGouMusicApi/app_win.exe";

    QString exePath;
    for (const QString &candidate : exeCandidates) {
        QFileInfo file(candidate);
        if (file.isFile()) {
            exePath = candidate;
            break;
        }
    }

    if (!exePath.isEmpty()) {
        QFileInfo exeInfo(exePath);
        kugou_server_process_->setWorkingDirectory(exeInfo.absolutePath());
        kugou_server_process_->start(exePath, QStringList());
        kugou_server_process_->setProcessChannelMode(QProcess::ProcessChannelMode::SeparateChannels);
    }
}

KuGouMusic::~KuGouMusic() {
    if (kugou_server_process_ && kugou_server_process_->state() != QProcess::NotRunning) {
        kugou_server_process_->kill();
        kugou_server_process_->waitForFinished(1000);
    }
}

void KuGouMusic::importMusic(const QString &playlist_link) {
    // 解析歌单ID
    QRegularExpression regex("Dcollection(.*?)%", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match = regex.match(playlist_link);
    QString playlist_id;
    if (match.hasMatch()) {
        playlist_id = "collection" + match.captured(1);
    } else {
        emit errorOccurred("歌单链接好像有问题哦，请检查一下😉");
        return;
    }

    // 清空上一次的解析数据
    songs_.clear();
    process_num_ = 0;
    emit songsNumberChanged(1, 0);

    fetchPlaylist(playlist_id);
}

void KuGouMusic::fetchPlaylist(const QString &playlist_id) {
    QString url = QString("http://127.0.0.1:3000/playlist/track/all?id=%1&pagesize=1000").arg(playlist_id);
    QNetworkRequest request;
    request.setUrl(QUrl(url));

    QNetworkReply *reply = network_manager_->get(request);
    connect(reply, &QNetworkReply::finished, this, &KuGouMusic::onPlaylistReplyFinished);
}

void KuGouMusic::onPlaylistReplyFinished() {
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) return;

    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(QString("歌单数据获取失败，可能网络有点问题 😉"));
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    reply->deleteLater();

    if (!doc.isObject()) {
        emit errorOccurred("歌单数据解析失败😰");
        return;
    }

    QJsonObject root = doc.object();
    QJsonArray songs_hash = root["data"].toObject()["songs"].toArray();
    songs_.resize(songs_hash.size());
    for (int i = 0; i < songs_.size(); ++i) {
        QString hash = songs_hash[i].toObject()["hash"].toString();
        fetchLyric(hash, i);
    }

    // emit songsNumberChanged(songs.size(), 0);

    // // 遍历每首歌，获取歌词
    // for (int i = 0; i < songs.size(); ++i) {
    //     QJsonObject song = songs[i].toObject();
    //     QString hash = song["hash"].toString();
    //     fetchLyric(hash, i);
    // }
}

void KuGouMusic::fetchLyric(const QString &hash, int song_index) {
    // 先搜索获取 accesskey
    QString accesskeyUrl = QString("http://127.0.0.1:3000/search/lyric?hash=%1").arg(hash);
    QNetworkRequest accesskeyRequest;
    accesskeyRequest.setUrl(QUrl(accesskeyUrl));
    QNetworkReply *reply = network_manager_->get(accesskeyRequest);
    connect(reply, &QNetworkReply::finished, [this, reply, song_index]() {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        reply->deleteLater();

        QJsonObject songInfo = doc.object()["candidates"].toArray()[0].toObject();
        QString songId = songInfo["id"].toString();
        QString accessKey = songInfo["accesskey"].toString();
        songs_[song_index].name = songInfo["song"].toString();
        songs_[song_index].singer = songInfo["singer"].toString();
        songs_[song_index].duration = songInfo["duration"].toInt();

        // 获取歌词
        QString lyricUrl =
            QString("http://127.0.0.1:3000/lyric?id=%1&accesskey=%2&fmt=krc&decode=true").arg(songId).arg(accessKey);
        QNetworkRequest lyricRequest;
        lyricRequest.setUrl(QUrl(lyricUrl));

        QNetworkReply *lyricReply = network_manager_->get(lyricRequest);
        connect(lyricReply, &QNetworkReply::finished, [this, lyricReply, song_index]() {
            QByteArray lyricData = lyricReply->readAll();
            QJsonDocument lyricDoc = QJsonDocument::fromJson(lyricData);
            lyricReply->deleteLater();

            QString lyricAll = lyricDoc.object()["decodeContent"].toString();
            QStringList lines = lyricAll.split("\n", Qt::SkipEmptyParts);
            for (const QString &line : lines) {
                if (line.startsWith("[id:")) continue;
                if (line.startsWith("[ar:")) continue;
                if (line.startsWith("[ti:")) continue;
                if (line.startsWith("[by:")) continue;
                if (line.startsWith("[hash:")) continue;
                if (line.startsWith("[al:")) continue;
                if (line.startsWith("[sign:")) continue;
                if (line.startsWith("[qq:")) continue;
                if (line.startsWith("[total:")) continue;
                if (line.startsWith("[offset:")) continue;

                // 解析翻译和音译
                if (line.startsWith("[language:")) {
                    parseTransLyric(line, song_index);
                    continue;
                }

                // 解析原始歌词
                parseLyric(line, song_index);
            }

            process_num_++;
            emit songsNumberChanged(songs_.size(), process_num_);
            if (process_num_ == songs_.size()) {
                emit taskFinished();
            }
        });
    });
}

void KuGouMusic::parseLyric(const QString &content, int song_index) {
    QRegularExpression re("^\\[(\\d+),(\\d+)\\](.*)$");
    QRegularExpressionMatch match = re.match(content);

    if (!match.hasMatch()) return;

    QString one_lien = match.captured(3);
    QRegularExpression tagRe("<\\d+,\\d+,\\d+>");
    QString lyric = one_lien.remove(tagRe).trimmed();
    songs_[song_index].lyric.push_back(lyric);
}

void KuGouMusic::parseTransLyric(const QString &content, int song_index) {
    // 提取 Base64 数据
    QRegularExpression re("\\[language:(.+)\\]");
    QRegularExpressionMatch match = re.match(content);
    QString base64Data = match.captured(1);
    QByteArray jsonData = QByteArray::fromBase64(base64Data.toUtf8());
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);

    QJsonObject obj = doc.object();
    QJsonArray lyrics = obj["content"].toArray();

    for (const QJsonValue &lyric : lyrics) {
        QJsonObject item = lyric.toObject();
        // 翻译
        if (item["type"] == 1) {
            QJsonArray lines = item["lyricContent"].toArray();
            if (lines.size() == 0) {
                continue;
            }
            for (int i = 0; i < lines.size(); ++i) {
                QJsonArray one_line = lines[i].toArray();
                QString line;
                for (int j = 0; j < one_line.size(); ++j) {
                    line += one_line[j].toString();
                }
                line = line.trimmed();
                songs_[song_index].tLyric.push_back(line);
            }
            songs_[song_index].has_tLyric = true;
        } else if (item["type"] == 0) {  // 音译
            QJsonArray lines = item["lyricContent"].toArray();
            if (lines.size() == 0) {
                continue;
            }
            for (int i = 0; i < lines.size(); ++i) {
                QJsonArray one_line = lines[i].toArray();
                QString line;
                for (int j = 0; j < one_line.size(); ++j) {
                    line += one_line[j].toString();
                }
                line = line.trimmed();
                songs_[song_index].rLyric.push_back(line);
            }
            songs_[song_index].has_rLyric = true;
        }
    }
}

}  // namespace LCX::core