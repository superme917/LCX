#ifndef BASE_MUSIC_H
#define BASE_MUSIC_H

#include <QWidget>
#include <QRegularExpression>
#include <execution>
#include <mutex>

const QMap<QChar, QChar> symbolMap = {
    {QChar(0x2019), '\''},   // 智能单引号 → 普通单引号
    {QChar(0x201C), '"'},     // 左双引号
    {QChar(0x201D), '"'},     // 右双引号
    {QChar(0xFF0D), '-'},     // 全角连字符
};

struct Song
{
    Song() {}

    QVector<QString> lyric;
    QVector<int> timeIndex;
    QVector<QString> tLyric;
    QVector<QString> rLyric;
    QVector<int> time;
    bool showTranslate = false;
    bool showRTranslate = false;
};

class BaseMusic : public QObject
{
    Q_OBJECT

public:
    BaseMusic(QWidget *parent = nullptr);
    ~BaseMusic() = default;

    QString getNumberFromStr(const QString &str) const;
    const QVector<Song> &getSongs() const;
    const QVector<QStringList> &getSongsList() const;
    QVector<Song> &songs();
    bool isValidLyric(QString content);
    int longestCommonSubstring(const QString &s1, const QString &s2);
    int timeToMilliseconds(const QString &timeStr);

    virtual bool checkMusicLink(QString musicLink) = 0;
    virtual void importMusic() = 0;

signals:
    void errorOccurred(QString error);
    void songsNumberChanged(int totalNum, int currNum);
    void taskFinished();

protected:
    QVector<QStringList> songsList_;
    QVector<Song> songs_;
    QString id_;
    int currSongIndex_ = 0;
    int songsNum_ = 0;
    QRegularExpression tRe_;
    QRegularExpression atRe_;
    std::mutex mtx;
};
#endif // BASE_MUSIC_H
