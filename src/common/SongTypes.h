#ifndef SONGTYPES_H
#define SONGTYPES_H

#include <QString>
#include <QVector>

// 歌曲信息
struct Song {
    Song() {}
    QString singer;               // 歌手
    QString name;                 // 歌名
    int duration;                 // 歌曲时长(ms)
    bool has_tLyric = false;      // 是否有翻译歌词
    bool has_rLyric = false;      // 是否有音译歌词
    QVector<QString> lyric;       // 歌词
    QVector<QString> tLyric;      // 翻译歌词
    QVector<QString> rLyric;      // 音译歌词
    QVector<int> time;            // 歌词时间戳(ms)
    bool showTranslate = false;   // 导出时是否需要翻译
    bool showRTranslate = false;  // 导出时是否需要音译
};

#endif
