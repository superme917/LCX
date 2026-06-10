//
// Create By WangYiFan on 2026/06/06
//

#pragma once

#include "core/base_music.h"

#include <QByteArray>
#include <QJsonObject>
#include <QString>

namespace LCX::core {

class DeepSeekClient {
public:
    struct Options {
        QString apiKey;
        QString apiUrl{QStringLiteral("https://api.deepseek.com/chat/completions")};
        QString model{QStringLiteral("deepseek-v4-pro")};
        int timeoutMs{60000};
        int maxRetries{2};
        int retryDelayMs{1000};
        double temperature{0.0};
    };

public:
    explicit DeepSeekClient(Options options);

    bool cleanSong(const Song &song, Song *cleanedSong, QString *error);

private:
    bool requestCleanSongOnce(const Song &song, Song *cleanedSong, QString *error);
    QJsonObject buildRequest(const Song &song) const;
    bool parseResponse(const QByteArray &data, const Song &sourceSong, Song *cleanedSong, QString *error) const;

private:
    Options options_;
};

}  // namespace LCX::core
