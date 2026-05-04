//
// Create By WangYiFan on 2026/04/30
//

#pragma once

#include <QRegularExpression>
#include <QWidget>
#include <atomic>
#include <execution>

namespace LCX::core {

const QMap<QChar, QChar> symbolMap = {
    {QChar(0x2019), '\''},  // 智能单引号 → 普通单引号
    {QChar(0x201C), '"'},   // 左双引号
    {QChar(0x201D), '"'},   // 右双引号
    {QChar(0xFF0D), '-'},   // 全角连字符
};

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

// 各大音乐平台歌词解析基类
class BaseMusic : public QObject {
    Q_OBJECT

public:
    // 构造函数
    BaseMusic(QWidget *parent = nullptr);
    // 析构函数
    ~BaseMusic();
    // 获取解析到的歌曲全信息
    const QVector<Song> &Songs() const;
    // 获取解析到的歌曲全信息，可修改
    QVector<Song> &Songs();
    // 解析歌单，导入所有歌曲信息
    virtual void importMusic(const QString &playlist_link) = 0;

protected:
    // TODO:是否为有效歌词，用于滤除冗余信息
    bool isValidLyric(QString content);
    // 文本时间戳转数字时间戳(ms)
    int timeToMilliseconds(const QString &timeStr);

signals:
    // 歌单解析错误信号
    void errorOccurred(const QString &error);
    // 用于显示歌单解析进度
    void songsNumberChanged(int totalNum, int currNum);
    // 歌单解析完成信号
    void taskFinished();

protected:
    QVector<Song> songs_;              // 歌单下的所有歌曲
    QString platlist_id_;              // 歌单ID
    QRegularExpression time_regular_;  // 匹配歌词时间戳
    std::atomic_int process_;
};

}  // namespace LCX::core
