#include "core/cloud_music.h"
#include <QVariantMap>
#include <QDateTime>
#include <QMutex>
#include <QJsonDocument>

CloudMusic::CloudMusic(QWidget *parent) : BaseMusic(parent), cloud_music_(new NeteaseCloudMusicApi(this))
{
}

bool CloudMusic::checkMusicLink(QString musicLink)
{
    QString id = getNumberFromStr(musicLink.mid(musicLink.indexOf("id") + 3));
    QString creatorId = getNumberFromStr(musicLink.mid(musicLink.indexOf("creatorId") + 10));

    if (id == "" || creatorId == "") {
        emit errorOccurred("歌单链接格式异常！");
        return false;
    }
    id_ = id;
    return true;
}

void CloudMusic::importMusic()
{
    songs_.clear();
    songsList_.clear();
    currSongIndex_ = 0;
    emit songsNumberChanged(1, 0);

    QVariantMap cloud_music = cloud_music_->playlist_track_all({{"id", id_}});
    QList<QVariant> songs = cloud_music["body"].toMap()["songs"].toList();
    songsNum_ = songs.size();
    songs_.resize(songsNum_);
    songsList_.resize(songsNum_);
    const QVariant* base = songs.begin();
    std::for_each(std::execution::par_unseq, songs.begin(), songs.end(), [&](const QVariant& song){
        QList<QVariant> authors = song.toMap()["ar"].toList();
        QString name = song.toMap()["name"].toString();
        QString song_id = song.toMap()["id"].toString();
        quint32 dt = song.toMap()["dt"].toUInt();
        QString authorsStr;
        for (auto ar : authors) {
            authorsStr += ar.toMap()["name"].toString() + "/";
        }
        authorsStr = authorsStr.left(authorsStr.size() - 1);

        int index = &song - base;
        songs_[index].lyric.emplace_back(name);
        songs_[index].lyric.emplace_back(authorsStr);
        songs_[index].time.emplace_back(INT32_MIN);
        songs_[index].time.emplace_back(INT32_MIN);
        songs_[index].tLyric.emplace_back("[]");
        songs_[index].tLyric.emplace_back("[]");
        songs_[index].rLyric.emplace_back("[]");
        songs_[index].rLyric.emplace_back("[]");

        QVariantMap cloudMusic = cloud_music_->lyric({{"id", song_id}});
        auto mapInfo = cloudMusic["body"].toMap();
        QString lyric = mapInfo["lrc"].toMap()["lyric"].toString();
        QString tLyric = mapInfo["tlyric"].toMap()["lyric"].toString();
        QString rLyric = mapInfo["romalrc"].toMap()["lyric"].toString();

        // lyric
        QStringList lines = lyric.split('\n', Qt::SkipEmptyParts);
        std::vector<std::pair<int, QString>> unsyncLyric;
        for (const QString& line : lines) {
            QString content = line;
            content.remove(tRe_);
            content.trimmed();
            if (content.isEmpty())
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
            songs_[index].lyric.emplace_back(lrc.second);
            songs_[index].time.emplace_back(lrc.first);
            songs_[index].tLyric.emplace_back("[]");
            songs_[index].rLyric.emplace_back("[]");
        }

        // tLyric
        lines = tLyric.split('\n', Qt::SkipEmptyParts);
        for (const QString& line : lines) {
            QString content = line;
            content.remove(tRe_);
            content.trimmed();
            if (content.isEmpty())
                continue;
            for (auto it = symbolMap.constBegin(); it != symbolMap.constEnd(); ++it) {
                content.replace(it.key(), it.value());
            }

            QRegularExpressionMatchIterator timeIt = tRe_.globalMatch(line);
            while (timeIt.hasNext()) {
                QRegularExpressionMatch timeMatch = timeIt.next();
                int ms = timeToMilliseconds(timeMatch.captured(1));
                if (ms != -1) {
                    for (int i = 0, j = songs_[index].lyric.size(); i < j; ++i) {
                        int delta = std::abs(ms - songs_[index].time[i]);
                        if (delta <= 1) {
                            songs_[index].tLyric[i] = content;
                        }
                    }
                }
            }
        }

        // rLyric
        lines = rLyric.split('\n', Qt::SkipEmptyParts);
        for (const QString& line : lines) {
            QString content = line;
            content.remove(tRe_);
            content.trimmed();
            if (content.isEmpty())
                continue;
            for (auto it = symbolMap.constBegin(); it != symbolMap.constEnd(); ++it) {
                content.replace(it.key(), it.value());
            }

            QRegularExpressionMatchIterator timeIt = tRe_.globalMatch(line);
            while (timeIt.hasNext()) {
                QRegularExpressionMatch timeMatch = timeIt.next();
                int ms = timeToMilliseconds(timeMatch.captured(1));
                if (ms != -1) {
                    for (int i = 0, j = songs_[index].lyric.size(); i < j; ++i) {
                        int delta = std::abs(ms - songs_[index].time[i]);
                        if (delta <= 1) {
                            songs_[index].rLyric[i] = content;
                        }
                    }
                }
            }
        }

        QStringList songList;
        songList << QString::number(index + 1) << name << authorsStr << QDateTime::fromMSecsSinceEpoch(dt).toString("m:ss") << (tLyric.isEmpty() ? "⛔️" : "❌") << (rLyric.isEmpty() ? "⛔️" : "❌");
        songsList_[index] = songList;

        int progress = songsNum_;
        {
            std::lock_guard<std::mutex> lock(mtx);
            for (const QStringList& str : songsList_) {
                if (str.isEmpty())
                    progress--;
            }
            emit songsNumberChanged(songsNum_, progress);
        }
    });

    emit taskFinished();
}
