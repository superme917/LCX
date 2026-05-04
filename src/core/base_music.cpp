//
// Create By WangYiFan on 2026/04/30
//

#include "core/base_music.h"

namespace LCX::core {

BaseMusic::BaseMusic(QWidget *parent)
    : QObject(parent), time_regular_(R"(\[((\d+):(\d+)\.(\d+))\])", QRegularExpression::CaseInsensitiveOption) {}

BaseMusic::~BaseMusic() {}

const QVector<Song> &BaseMusic::Songs() const { return songs_; }

QVector<Song> &BaseMusic::Songs() { return songs_; }

bool BaseMusic::isValidLyric(QString content) { return true; }

int BaseMusic::timeToMilliseconds(const QString &timeStr) {
    QRegularExpressionMatch match = time_regular_.match(timeStr);

    if (match.hasMatch()) {
        int minutes = match.captured(2).toInt();
        int seconds = match.captured(3).toInt();
        QString smilliseconds = match.captured(4);
        while (smilliseconds.size() <= 2) smilliseconds += '0';
        int milliseconds = smilliseconds.toInt();

        return (minutes * 60 + seconds) * 1000 + milliseconds;
    }
    return -1;  // 无效格式
}

}  // namespace LCX::core