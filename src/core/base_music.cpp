//
// Create By WangYiFan on 2026/04/30
//

#include "core/base_music.h"
#include "core/deepseek_lyric_cleaner.h"

namespace LCX::core {

BaseMusic::BaseMusic(QWidget *parent)
    : QObject(parent),
      time_regular_(R"(\[((\d+):(\d+)\.(\d+)(?:-\d+)?)\])", QRegularExpression::CaseInsensitiveOption),
      lyric_cleaner_(new DeepSeekLyricCleaner(this)),
      deepseek_api_key_(QStringLiteral("sk-4303e19e3e384d13909f5d5bb1c4939b")) {
    connect(lyric_cleaner_, &DeepSeekLyricCleaner::progressChanged, this,
            [this](int total, int current) { emit songsNumberChanged(total, current); });
    connect(lyric_cleaner_, &DeepSeekLyricCleaner::errorOccurred, this,
            [this](int, const QString &error) { emit errorOccurred(error); });
    connect(lyric_cleaner_, &DeepSeekLyricCleaner::finished, this,
            [this](bool, const QStringList &) { emit taskFinished(); });
}

BaseMusic::~BaseMusic() {}

const QVector<Song> &BaseMusic::Songs() const { return songs_; }

QVector<Song> &BaseMusic::Songs() { return songs_; }

void BaseMusic::setLyricCleanupEnabled(bool enabled) { lyric_cleanup_enabled_ = enabled; }

void BaseMusic::setDeepSeekApiKey(const QString &apiKey) { deepseek_api_key_ = apiKey; }

void BaseMusic::setDeepSeekMaxConcurrency(int maxConcurrency) {
    deepseek_max_concurrency_ = std::max(1, maxConcurrency);
}

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

void BaseMusic::finishParsing() {
    if (!lyric_cleanup_enabled_ || songs_.isEmpty() || deepseek_api_key_.trimmed().isEmpty()) {
        emit taskFinished();
        return;
    }

    if (lyric_cleaner_->isRunning()) {
        return;
    }

    DeepSeekLyricCleaner::Options options;
    options.apiKey = deepseek_api_key_;
    options.maxRetries = 2;
    options.retryDelayMs = 1000;
    options.temperature = 0.0;

    lyric_cleaner_->start(&songs_, options, deepseek_max_concurrency_);
}

}  // namespace LCX::core