//
// Create By WangYiFan on 2026/06/06
//

#pragma once

#include "core/deepseek_client.h"

#include <QMutex>
#include <QObject>
#include <QThreadPool>
#include <QStringList>

#include <atomic>

namespace LCX::core {

class DeepSeekLyricCleaner : public QObject {
    Q_OBJECT

public:
    using Options = DeepSeekClient::Options;

public:
    explicit DeepSeekLyricCleaner(QObject *parent = nullptr);
    ~DeepSeekLyricCleaner() override;

    bool isRunning() const;

public slots:
    // songs 必须在清洗任务结束前保持有效。成功清洗的歌曲会按原索引回写。
    void start(QVector<Song> *songs, const Options &options, int maxConcurrency = 4);
    void cancel();

signals:
    void progressChanged(int total, int current);
    void songCleaned(int songIndex);
    void errorOccurred(int songIndex, const QString &error);
    void finished(bool success, const QStringList &errors);

private:
    void finishOne(int songIndex, bool success, const QString &error);

private:
    mutable QMutex mutex_;
    QThreadPool pool_;
    QVector<Song> *songs_{nullptr};
    QStringList errors_;
    std::atomic_bool running_{false};
    std::atomic_bool canceled_{false};
    std::atomic_int total_{0};
    std::atomic_int completed_{0};
    std::atomic_int failed_{0};
};

}  // namespace LCX::core
