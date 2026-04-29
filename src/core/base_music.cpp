#include "core/base_music.h"

BaseMusic::BaseMusic(QWidget *parent)
    : QObject(parent)
    , tRe_(R"(\[(\d+:\d+\.\d+)\])", QRegularExpression::CaseInsensitiveOption)
    , atRe_(R"((\d+):(\d+)\.(\d+))", QRegularExpression::CaseInsensitiveOption)
{
}

QString BaseMusic::getNumberFromStr(const QString &str) const
{
    QString res;
    for (int i = 0; i < str.size(); ++i) {
        if (str[i] >= '0' && str[i] <= '9') {
            res += str[i];
        } else {
            break;
        }
    }
    return res;
}

const QVector<Song> &BaseMusic::getSongs() const
{
    return songs_;
}

const QVector<QStringList> &BaseMusic::getSongsList() const
{
    return songsList_;
}

QVector<Song> &BaseMusic::songs()
{
    return songs_;
}

bool BaseMusic::isValidLyric(QString content)
{
    if (content.isEmpty() || content == "//")
        return false;
    return true;
}

// bool BaseMusic::isSongInfo(const QString lyric, const QString &name)
// {
//     if (lyric.contains("男：") || lyric.contains("女：") || lyric.contains("合："))
//         return false;
//     else if (songs_.back().lyric.size() == 1 && lyric.contains(name))
//         return true;
//     else if (lyric.contains("此版本") || lyric.contains("版权") || lyric.contains("@") || lyric.contains("联合出品") || lyric.contains("本歌曲来自") || lyric.contains("千亿流量扶持") || lyric.contains("演唱：") || lyric.contains("不得翻唱"))
//         return true;
//     else if (lyric.contains(":")) {
//         if (lyric.indexOf(":") == lyric.size() - 1)
//             return false;
//         return true;
//     } else if (lyric.contains("：")) {
//         if (lyric.indexOf("：") == lyric.size() - 1)
//             return false;
//         return true;
//     }
//     return false;
// }

int BaseMusic::longestCommonSubstring(const QString &s1, const QString &s2) {
    int m = s1.length(), n = s2.length();
    QVector<QVector<int>> dp(m + 1, QVector<int>(n + 1, 0));
    int max_len = 0;

    for (int i = 1; i <= m; ++i) {
        for (int j = 1; j <= n; ++j) {
            if (s1[i-1] == s2[j-1]) { // 注意下标从 0 开始
                dp[i][j] = dp[i-1][j-1] + 1;
                max_len = qMax(max_len, dp[i][j]);
            } else {
                dp[i][j] = 0;
            }
        }
    }
    return max_len;
}

int BaseMusic::timeToMilliseconds(const QString &timeStr)
{
    QRegularExpressionMatch match = atRe_.match(timeStr);

    if (match.hasMatch()) {
        int minutes = match.captured(1).toInt();
        int seconds = match.captured(2).toInt();
        QString smilliseconds = match.captured(3);
        while (smilliseconds.size() <= 2)
            smilliseconds += '0';
        int milliseconds = smilliseconds.toInt();

        return (minutes * 60 + seconds) * 1000 + milliseconds;
    }
    return -1; // 无效格式
}
