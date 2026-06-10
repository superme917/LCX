//
// Create By WangYiFan on 2026/06/06
//

#include "core/deepseek_client.h"

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>

#include <algorithm>
#include <utility>

namespace LCX::core {
namespace {

QJsonArray toJsonArray(const QVector<QString> &lines) {
    QJsonArray array;
    for (const QString &line : lines) {
        array.append(line);
    }
    return array;
}

QVector<QString> toStringVector(const QJsonValue &value, const QVector<QString> &fallback) {
    if (!value.isArray()) {
        return fallback;
    }

    QVector<QString> lines;
    const QJsonArray array = value.toArray();
    lines.reserve(array.size());
    for (const QJsonValue &item : array) {
        const QString line = item.toString().trimmed();
        if (!line.isEmpty() && line != QStringLiteral("//")) {
            lines.push_back(line);
        }
    }
    return lines;
}

QVector<int> toRemoveIndexes(const QJsonValue &value, int lineCount) {
    QVector<int> indexes;
    if (!value.isArray()) {
        return indexes;
    }

    const QJsonArray array = value.toArray();
    indexes.reserve(array.size());
    for (const QJsonValue &item : array) {
        const int index = item.toInt(-1);
        if (index >= 0 && index < lineCount) {
            indexes.push_back(index);
        }
    }

    std::sort(indexes.begin(), indexes.end());
    indexes.erase(std::unique(indexes.begin(), indexes.end()), indexes.end());
    return indexes;
}

QVector<QString> removeStringIndexes(const QVector<QString> &lines, const QVector<int> &removeIndexes) {
    QVector<QString> result;
    result.reserve(lines.size());

    int removeCursor = 0;
    for (int i = 0; i < lines.size(); ++i) {
        if (removeCursor < removeIndexes.size() && removeIndexes[removeCursor] == i) {
            ++removeCursor;
            continue;
        }
        result.push_back(lines[i]);
    }
    return result;
}

QVector<int> removeTimeIndexes(const QVector<int> &times, const QVector<int> &removeIndexes) {
    QVector<int> result;
    result.reserve(times.size());

    int removeCursor = 0;
    for (int i = 0; i < times.size(); ++i) {
        if (removeCursor < removeIndexes.size() && removeIndexes[removeCursor] == i) {
            ++removeCursor;
            continue;
        }
        result.push_back(times[i]);
    }
    return result;
}

QString stripMarkdownFence(QString text) {
    text = text.trimmed();
    if (!text.startsWith(QStringLiteral("```"))) {
        return text;
    }

    const int firstLineEnd = text.indexOf('\n');
    const int lastFence = text.lastIndexOf(QStringLiteral("```"));
    if (firstLineEnd < 0 || lastFence <= firstLineEnd) {
        return text;
    }
    return text.mid(firstLineEnd + 1, lastFence - firstLineEnd - 1).trimmed();
}

QJsonObject extractJsonObject(QString text, QString *error) {
    text = stripMarkdownFence(text);

    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(text.toUtf8(), &parseError);
    if (!document.isObject()) {
        const int begin = text.indexOf('{');
        const int end = text.lastIndexOf('}');
        if (begin >= 0 && end > begin) {
            document = QJsonDocument::fromJson(text.mid(begin, end - begin + 1).toUtf8(), &parseError);
        }
    }

    if (!document.isObject()) {
        if (error) {
            *error = QStringLiteral("DeepSeek returned invalid JSON: %1").arg(parseError.errorString());
        }
        return {};
    }
    return document.object();
}

}  // namespace

DeepSeekClient::DeepSeekClient(Options options) : options_(std::move(options)) {}

bool DeepSeekClient::cleanSong(const Song &song, Song *cleanedSong, QString *error) {
    if (!cleanedSong) {
        if (error) {
            *error = QStringLiteral("cleanedSong is null");
        }
        return false;
    }
    if (options_.apiKey.trimmed().isEmpty()) {
        if (error) {
            *error = QStringLiteral("DeepSeek API Key is empty");
        }
        return false;
    }

    QString lastError;
    const int attempts = std::max(1, options_.maxRetries + 1);
    for (int attempt = 1; attempt <= attempts; ++attempt) {
        QString currentError;
        if (requestCleanSongOnce(song, cleanedSong, &currentError)) {
            return true;
        }

        lastError = currentError;
        if (attempt < attempts && options_.retryDelayMs > 0) {
            QEventLoop loop;
            QTimer::singleShot(options_.retryDelayMs, &loop, &QEventLoop::quit);
            loop.exec();
        }
    }

    if (error) {
        *error = attempts > 1
                     ? QStringLiteral("DeepSeek request failed after %1 attempts: %2").arg(attempts).arg(lastError)
                     : lastError;
    }
    return false;
}

bool DeepSeekClient::requestCleanSongOnce(const Song &song, Song *cleanedSong, QString *error) {
    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl(options_.apiUrl));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setRawHeader("Authorization", QStringLiteral("Bearer %1").arg(options_.apiKey).toUtf8());

    QNetworkReply *reply = manager.post(request, QJsonDocument(buildRequest(song)).toJson(QJsonDocument::Compact));

    QTimer timer;
    timer.setSingleShot(true);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, reply, &QNetworkReply::abort);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(options_.timeoutMs);
    loop.exec();

    if (!timer.isActive()) {
        reply->deleteLater();
        if (error) {
            *error = QStringLiteral("DeepSeek request timed out");
        }
        return false;
    }
    timer.stop();

    const QByteArray response = reply->readAll();
    const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QNetworkReply::NetworkError networkError = reply->error();
    const QString networkErrorText = reply->errorString();
    reply->deleteLater();

    if (networkError != QNetworkReply::NoError) {
        if (error) {
            *error = QStringLiteral("DeepSeek network error: %1").arg(networkErrorText);
        }
        return false;
    }
    if (httpStatus < 200 || httpStatus >= 300) {
        if (error) {
            *error = QStringLiteral("DeepSeek HTTP error %1: %2").arg(httpStatus).arg(QString::fromUtf8(response));
        }
        return false;
    }

    return parseResponse(response, song, cleanedSong, error);
}

QJsonObject DeepSeekClient::buildRequest(const Song &song) const {
    QJsonObject songObject;
    songObject.insert(QStringLiteral("name"), song.name);
    songObject.insert(QStringLiteral("singer"), song.singer);
    songObject.insert(QStringLiteral("lyric"), toJsonArray(song.lyric));
    songObject.insert(QStringLiteral("tLyric"), toJsonArray(song.tLyric));
    songObject.insert(QStringLiteral("rLyric"), toJsonArray(song.rLyric));

    const QString systemPrompt = QStringLiteral(
        "You are a lyric cleanup classifier. Your only task is to identify original lyric array indexes "
        "that are definitely non-lyric metadata or noise. To avoid false deletion, use this rule: when uncertain, keep "
        "the line. Never rewrite, translate, merge, split, normalize, or reorder any text. Return indexes only.");
    const QString userPrompt =
        QStringLiteral(
            "Classify removable lines from song.lyric only.\n"
            "Delete an index only if the line is clearly one of these non-lyric items:\n"
            "- lyricist/composer/arranger/producer/mixer/mastering/publisher credits\n"
            "- copyright/license/publishing/company/distribution notices\n"
            "- platform source, uploader, watermark, website, app, or karaoke service text\n"
            "- empty placeholders such as //, instrumental/no-lyric notices, or unavailable lyric notices\n"
            "- translation/source notes that are not actual sung lyrics\n"
            "Do NOT delete real sung lyric lines, repeated chorus lines, ad-libs, interjections, "
            "punctuation-only sung effects, or any line that could plausibly be part of the song. "
            "If a line contains words like lyricist/composer/copyright but may be sung as a real "
            "sentence, keep it.\n"
            "Return strict JSON only, no Markdown. Required schema: "
            "{\"removeIndexes\":[0,1],\"reason\":{\"0\":\"credit line\"}}.\n"
            "Input JSON:\n%1")
            .arg(QString::fromUtf8(QJsonDocument(songObject).toJson(QJsonDocument::Compact)));

    QJsonArray messages;
    messages.append(
        QJsonObject{{QStringLiteral("role"), QStringLiteral("system")}, {QStringLiteral("content"), systemPrompt}});
    messages.append(
        QJsonObject{{QStringLiteral("role"), QStringLiteral("user")}, {QStringLiteral("content"), userPrompt}});

    QJsonObject request;
    request.insert(QStringLiteral("model"), options_.model);
    request.insert(QStringLiteral("messages"), messages);
    request.insert(QStringLiteral("temperature"), options_.temperature);
    request.insert(QStringLiteral("stream"), false);
    return request;
}

bool DeepSeekClient::parseResponse(const QByteArray &data, const Song &sourceSong, Song *cleanedSong,
                                   QString *error) const {
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(data, &parseError);
    if (!document.isObject()) {
        if (error) {
            *error = QStringLiteral("DeepSeek response JSON parse failed: %1").arg(parseError.errorString());
        }
        return false;
    }

    const QJsonArray choices = document.object().value(QStringLiteral("choices")).toArray();
    if (choices.isEmpty()) {
        if (error) {
            *error = QStringLiteral("DeepSeek response has no choices");
        }
        return false;
    }

    const QString content = choices.first()
                                .toObject()
                                .value(QStringLiteral("message"))
                                .toObject()
                                .value(QStringLiteral("content"))
                                .toString();
    const QJsonObject result = extractJsonObject(content, error);
    if (result.isEmpty()) {
        return false;
    }

    const QVector<int> removeIndexes =
        toRemoveIndexes(result.value(QStringLiteral("removeIndexes")), sourceSong.lyric.size());
    if (removeIndexes.size() == sourceSong.lyric.size() && !sourceSong.lyric.isEmpty()) {
        if (error) {
            *error = QStringLiteral("DeepSeek tried to remove all lyric lines; cleanup rejected");
        }
        return false;
    }

    Song updated = sourceSong;
    updated.lyric = removeStringIndexes(sourceSong.lyric, removeIndexes);
    if (sourceSong.time.size() == sourceSong.lyric.size()) {
        updated.time = removeTimeIndexes(sourceSong.time, removeIndexes);
    }
    if (sourceSong.tLyric.size() == sourceSong.lyric.size()) {
        updated.tLyric = removeStringIndexes(sourceSong.tLyric, removeIndexes);
    } else {
        updated.tLyric = toStringVector(result.value(QStringLiteral("tLyric")), sourceSong.tLyric);
    }
    if (sourceSong.rLyric.size() == sourceSong.lyric.size()) {
        updated.rLyric = removeStringIndexes(sourceSong.rLyric, removeIndexes);
    } else {
        updated.rLyric = toStringVector(result.value(QStringLiteral("rLyric")), sourceSong.rLyric);
    }
    updated.has_tLyric = !updated.tLyric.isEmpty();
    updated.has_rLyric = !updated.rLyric.isEmpty();

    *cleanedSong = updated;
    return true;
}

}  // namespace LCX::core
