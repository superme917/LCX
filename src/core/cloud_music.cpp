#include "core/cloud_music.h"

#include <QDateTime>
#include <QJsonDocument>
#include <QMutex>
#include <QVariantMap>

namespace LCX::core {

CloudMusic::CloudMusic(QWidget *parent) : BaseMusic(parent), cloud_music_(new NeteaseCloudMusicApi(parent)) {}

CloudMusic::~CloudMusic() {}

void CloudMusic::importMusic(const QString &playlist_link) {
    // 获取歌单ID
    QRegularExpression id_regular("id=(\\d+)");
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

    // 解析歌单，获取歌单下的歌曲信息
    QVariantMap cloud_music = cloud_music_->playlist_track_all({{"id", playlist_id}});
    QList<QVariant> songs = cloud_music["body"].toMap()["songs"].toList();
    songs_.resize(songs.size());
    const QVariant *base = songs.begin();
    process_.store(0);
    // 歌词解析并行加速
    std::for_each(std::execution::seq, songs.begin(), songs.end(), [&](const QVariant &song) {
        // 获取歌曲信息，并填入songs_
        QList<QVariant> singer = song.toMap()["ar"].toList();  // 歌手
        QString name = song.toMap()["name"].toString();        // 歌名
        QString song_id = song.toMap()["id"].toString();       // 歌曲ID
        int duration = song.toMap()["dt"].toInt();             // 歌曲时长
        QString all_singer;
        for (auto ar : singer) {
            all_singer += ar.toMap()["name"].toString() + "/";
        }
        all_singer = all_singer.left(all_singer.size() - 1);

        int index = &song - base;
        songs_[index].singer = all_singer;
        songs_[index].name = name;
        songs_[index].duration = duration;

        // 解析歌曲，获取歌词信息
        QVariantMap cloudMusic = cloud_music_->lyric({{"id", song_id}});
        auto mapInfo = cloudMusic["body"].toMap();
        QString lyric = mapInfo["lrc"].toMap()["lyric"].toString();       // 原始歌词
        QString tLyric = mapInfo["tlyric"].toMap()["lyric"].toString();   // 翻译歌词
        QString rLyric = mapInfo["romalrc"].toMap()["lyric"].toString();  // 音译歌词

        // 解析原始歌词，填入songs_
        QStringList lines = lyric.split('\n', Qt::SkipEmptyParts);
        std::vector<std::pair<int, QString>> unsynced_lyric;
        for (const QString &line : lines) {
            QString content = line;
            content.remove(time_regular_);
            content.trimmed();
            if (content.isEmpty()) continue;
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
            songs_[index].lyric.emplace_back(lrc.second);
            songs_[index].time.emplace_back(lrc.first);
            songs_[index].tLyric.emplace_back("");
            songs_[index].rLyric.emplace_back("");
        }

        // 解析翻译歌词
        lines = tLyric.split('\n', Qt::SkipEmptyParts);
        for (const QString &line : lines) {
            QString content = line;
            content.remove(time_regular_);
            content.trimmed();
            if (content.isEmpty()) continue;
            for (auto it = symbolMap.constBegin(); it != symbolMap.constEnd(); ++it) {
                content.replace(it.key(), it.value());
            }

            // 通过时间戳对齐原始歌词和翻译歌词
            int ms = timeToMilliseconds(line);
            if (ms != -1) {
                for (int i = 0; i < songs_[index].lyric.size(); ++i) {
                    int delta = std::abs(ms - songs_[index].time[i]);
                    if (delta <= 1) {
                        songs_[index].tLyric[i] = content;
                    }
                }
            }
            songs_[index].has_tLyric = true;
        }

        // 解析音译歌词
        lines = rLyric.split('\n', Qt::SkipEmptyParts);
        for (const QString &line : lines) {
            QString content = line;
            content.remove(time_regular_);
            content.trimmed();
            if (content.isEmpty()) continue;
            for (auto it = symbolMap.constBegin(); it != symbolMap.constEnd(); ++it) {
                content.replace(it.key(), it.value());
            }

            // 通过时间戳对齐原始歌词和音译歌词
            int ms = timeToMilliseconds(line);
            if (ms != -1) {
                for (int i = 0, j = songs_[index].lyric.size(); i < j; ++i) {
                    int delta = std::abs(ms - songs_[index].time[i]);
                    if (delta <= 1) {
                        songs_[index].rLyric[i] = content;
                    }
                }
            }
            songs_[index].has_rLyric = true;
        }

        // 发送歌词解析进度信号
        process_.store(process_.load() + 1);
        emit songsNumberChanged(songs_.size(), process_);
    });
    // 发送任务完成信号
    emit taskFinished();
}

}  // namespace LCX::core
