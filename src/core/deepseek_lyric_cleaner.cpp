//
// Create By WangYiFan on 2026/06/06
//

#include "core/deepseek_lyric_cleaner.h"

#include <QMutexLocker>
#include <QRunnable>

#include <algorithm>

namespace LCX::core {

DeepSeekLyricCleaner::DeepSeekLyricCleaner(QObject *parent) : QObject(parent) {}

DeepSeekLyricCleaner::~DeepSeekLyricCleaner() {
    cancel();
    pool_.waitForDone();
}

bool DeepSeekLyricCleaner::isRunning() const { return running_.load(); }

void DeepSeekLyricCleaner::start(QVector<Song> *songs, const Options &options, int maxConcurrency) {
    if (running_.exchange(true)) {
        emit errorOccurred(-1, QStringLiteral("DeepSeek lyric clean task is already running"));
        return;
    }

    if (!songs || songs->isEmpty()) {
        running_.store(false);
        emit finished(true, QStringList{});
        return;
    }

    if (options.apiKey.trimmed().isEmpty()) {
        running_.store(false);
        emit finished(false, QStringList{QStringLiteral("DeepSeek API Key is empty")});
        return;
    }

    {
        QMutexLocker locker(&mutex_);
        songs_ = songs;
        errors_.clear();
    }

    canceled_.store(false);
    total_.store(songs->size());
    completed_.store(0);
    failed_.store(0);
    pool_.setMaxThreadCount(std::max(1, maxConcurrency));

    for (int i = 0; i < songs->size(); ++i) {
        pool_.start(QRunnable::create([this, i, options]() {
            if (canceled_.load()) {
                finishOne(i, false, QStringLiteral("Task canceled"));
                return;
            }

            Song sourceSong;
            bool invalidSongs = false;
            {
                QMutexLocker locker(&mutex_);
                if (!songs_ || i >= songs_->size()) {
                    invalidSongs = true;
                } else {
                    sourceSong = songs_->at(i);
                }
            }

            if (invalidSongs) {
                finishOne(i, false, QStringLiteral("Song list is invalid"));
                return;
            }

            DeepSeekClient client(options);
            Song cleanedSong;
            QString error;
            const bool ok = client.cleanSong(sourceSong, &cleanedSong, &error);

            if (ok && !canceled_.load()) {
                QMutexLocker locker(&mutex_);
                if (songs_ && i < songs_->size()) {
                    (*songs_)[i] = cleanedSong;
                }
            }

            finishOne(i, ok && !canceled_.load(), error);
        }));
    }
}

void DeepSeekLyricCleaner::cancel() { canceled_.store(true); }

void DeepSeekLyricCleaner::finishOne(int songIndex, bool success, const QString &error) {
    if (!success) {
        failed_.fetch_add(1);
        const QString message = error.isEmpty()
                                    ? QStringLiteral("Song %1 clean failed").arg(songIndex + 1)
                                    : QStringLiteral("Song %1 clean failed: %2").arg(songIndex + 1).arg(error);
        {
            QMutexLocker locker(&mutex_);
            errors_.push_back(message);
        }
        emit errorOccurred(songIndex, message);
    } else {
        emit songCleaned(songIndex);
    }

    const int current = completed_.fetch_add(1) + 1;
    const int total = total_.load();
    emit progressChanged(2 * total, total + current);

    if (current == total) {
        QStringList errors;
        {
            QMutexLocker locker(&mutex_);
            errors = errors_;
            songs_ = nullptr;
        }

        const bool ok = failed_.load() == 0 && !canceled_.load();
        running_.store(false);
        emit finished(ok, errors);
    }
}

}  // namespace LCX::core
