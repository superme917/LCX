//
// Create By WangYiFan on 2026/05/10
//

#include "core/qishui_music.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSslConfiguration>
#include <QTime>


namespace LCX::core {

QiShuiMusic::QiShuiMusic(QWidget *parent) : BaseMusic(parent), network_manager_(new QNetworkAccessManager(this)) {}

QiShuiMusic::~QiShuiMusic() {}
void QiShuiMusic::importMusic(const QString &playlist_link) {
    // 获取歌单ID
    QRegularExpression id_regular("playlist_id=(.*?)&");
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
    process_num_ = 0;
    emit songsNumberChanged(1, 0);

    QString url = QString("https://api.suol.cc/v1/music_qs.php?page=1000&action=playlist&id=%1&m_token=%2")
                      .arg(playlist_id)
                      .arg(token_);
    QNetworkRequest request;
    request.setUrl(QUrl(url));

    QNetworkReply *reply = network_manager_->QNetworkAccessManager::get(request);
    connect(reply, &QNetworkReply::finished, this, &QiShuiMusic::onPlaylistReplyFinished);
}

void QiShuiMusic::onPlaylistReplyFinished() {
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

    QJsonObject root = doc.object();
    QJsonArray songs_info = root["data"].toObject()["list"].toArray();
    songs_.resize(songs_info.size());
    for (int i = 0; i < songs_.size(); ++i) {
        fetchLyric(songs_info[i].toObject()["id"].toString(), i);
    }
}

void QiShuiMusic::fetchLyric(const QString &id, int song_index) {
    QString lyricUrl = QString("https://api.suol.cc/v1/music_qs.php?action=song&id=%1&m_token=%2").arg(id).arg(token_);
    QNetworkRequest lyricRequest;
    lyricRequest.setUrl(QUrl(lyricUrl));
    QNetworkReply *reply = network_manager_->get(lyricRequest);
    connect(reply, &QNetworkReply::finished, [this, reply, song_index]() {
        QByteArray lyricData = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(lyricData);
        reply->deleteLater();

        QJsonObject song = doc.object()["data"].toObject();
        songs_[song_index].name = song["name"].toString();
        songs_[song_index].singer = song["artists"].toString();
        QTime time = QTime::fromString(song["duration"].toString(), "mm:ss");
        songs_[song_index].duration = QTime(0, 0, 0).msecsTo(time);

        QString content = song["lyric_lrc"].toString();
        QStringList lines = content.split("\n", Qt::SkipEmptyParts);
        QRegularExpression lyric_regular(R"(\[.*?\](.*))");
        for (const QString line : lines) {
            QRegularExpressionMatch match = lyric_regular.match(line);
            if (match.isValid()) {
                songs_[song_index].lyric.push_back(match.captured(1));
            }
        }
        process_num_++;
        if (lyric_cleanup_enabled_) {
            emit songsNumberChanged(2 * songs_.size(), process_num_);
        } else {
            emit songsNumberChanged(songs_.size(), process_num_);
        }
        if (process_num_ == songs_.size()) {
            finishParsing();
        }
    });
}

}  // namespace LCX::core