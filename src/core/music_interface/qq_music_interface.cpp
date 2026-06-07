#include "core/music_interface/qq_music_interface.h"
#include "core/process/network_request.h"

#include <QJsonDocument>
#include <QRegularExpression>
#include <QStringList>

QQMusicInterface::QQMusicInterface(QObject *parent) : BaseMusicInterface(parent) {}

QQMusicInterface::~QQMusicInterface() {}

bool QQMusicInterface::ParsePlaylistId(const QString &playlist_id) {
    QRegularExpression regex("playlist/(\\d+)");
    QRegularExpressionMatch match = regex.match(playlist_id);
    if (match.hasMatch()) {
        playlist_id_ = match.captured(1);
        return true;
    } else {
        emit errorOccurred("歌单链接好像有问题哦，请检查一下😉");
        return false;
    }
}

void QQMusicInterface::ImportMusic() {
    QString songlist_urk =
        QStringLiteral("http://localhost:8080/songlist/%1/detail?num=1000&onlysong=true&userinfo=false")
            .arg(playlist_id_);
    NetworkRequest request;
    request.setHeader("Accept", "application/json");
    QByteArray data = request.get(songlist_urk);
    if (request.lastError() == NetworkRequest::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(data);
        // qDebug() << doc;
    } else {
        emit errorOccurred(request.errorString());
    }
}