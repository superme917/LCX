//
// Create By WangYiFan on 2026/04/30
//

#include "core/qq_music.h"
#include <QApplication>
#include <execution>
#include <mutex>
#include <QThreadPool>

#define USERAGENT "Mozilla/5.0 (Linux; Android 4.1.1; Nexus 7 Build/JRO03D) AppleWebKit/535.19 (KHTML, like Gecko) Chrome/18.0.1025.166  Safari/535.19"
#define PLAYLISTLINKHEAD "https://c.y.qq.com/qzone/fcg-bin/fcg_ucc_getcdinfo_byids_cp.fcg?type=1&json=1&utf8=1&onlysong=0&disstid="
#define PALYLISTLINKTAIL "&format=jsonp&g_tk=5381&jsonpCallback=playlistinfoCallback&loginUin=0&hostUin=0&format=jsonp&inCharset=utf8&outCharset=utf-8&notice=0&platform=yqq&needNewCode=0"
#define SINGLEGECHIHEAD "https://c.y.qq.com/lyric/fcgi-bin/fcg_query_lyric_new.fcg?callback=MusicJsonCallback_lrc&pcachetime="
#define SINGLEGECHIMID "&songmid="
#define SINGLEGECHITAIL "&g_tk=5381&jsonpCallback=MusicJsonCallback_lrc&loginUin=0&hostUin=0&format=jsonp&inCharset=utf8&outCharset=utf-8&notice=0&platform=yqq&needNewCode=0&lyricType=7"

QQMusic::QQMusic(QWidget *parent) : BaseMusic(parent)
{
    downloader_ = new downloader();
    connect(downloader_, &downloader::downloadError, this, &QQMusic::errorOccurred);
    connect(downloader_, &downloader::redirected, this, &QQMusic::errorOccurred);
    connect(downloader_, &downloader::finished, this, &QQMusic::dowmloadFinished);
}

bool QQMusic::checkMusicLink(QString musicLink)
{
    if (!musicLink.contains("y.qq.com")) {
        emit errorOccurred("歌单链接格式异常！");
        return false;
    }
    id_ = getNumberFromStr(musicLink.mid(musicLink.indexOf("playlist/") + 9));
    return true;
}

void QQMusic::importMusic()
{
    curIdx_ = 0;
    songsNum_ = 0;
    songs_.clear();
    currSongIndex_ = 0;
    songsList_.clear();
    emit songsNumberChanged(1, 0);

    QString refererString = QString("https://y.qq.com/n/yqq/playsquare/") + id_ + QString(".html");
    QString playListDownloadLink = QString(PLAYLISTLINKHEAD) + id_ + QString(PALYLISTLINKTAIL);
    downloader_->init(playListDownloadLink);
    downloader_->setReferer(refererString);
    downloader_->doGet();
}

void QQMusic::dowmloadFinished()
{
    songMidList_.clear();
    int index = 0;
    QString songmid;
    QString songname;
    QString singer;
    QString playListName;
    QString str = downloader_->readData().mid(21);
    str = str.left(str.size() - 1);
    QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8());
    QJsonArray array = doc["cdlist"].toArray();
    QJsonArray songlists = array[0].toObject()["songlist"].toArray();
    songsNum_ = songlists.size();
    songsList_.resize(songsNum_);
    songs_.resize(songsNum_);
    for (int i = 0; i < songsNum_; ++i) {
        QJsonObject songInfo = songlists[i].toObject();
        songmid = songInfo["songmid"].toString();
        songname = songInfo["songname"].toString();

        singer = "";
        QJsonArray singers = songInfo["singer"].toArray();
        for (int j = 0, jl = singers.size(); j < jl; ++j) {
            QJsonObject singerInfo = singers[j].toObject();
            singer += singerInfo["name"].toString();
            if (j != jl - 1)
                singer += "/";
        }

        songMidList_.append(songmid);
        songs_[i].lyric.emplace_back(songname);
        songs_[i].lyric.emplace_back(singer);
        songs_[i].time.emplace_back(INT32_MIN);
        songs_[i].time.emplace_back(INT32_MIN);
        songs_[i].tLyric.emplace_back("[]");
        songs_[i].tLyric.emplace_back("[]");
        songs_[i].rLyric.emplace_back("[]");
        songs_[i].rLyric.emplace_back("[]");
    }

    beginToDownloadSongs();
}

void QQMusic::beginToDownloadSongs()
{
    for (int i = 0; i < songMidList_.size(); ++i) {
        beginToDownloadSongsByQQ(i);
    }
    emit taskFinished();
}

void QQMusic::beginToDownloadSongsByQQ(int index)
{
    QEventLoop loop;
    QString songMid = songMidList_[index];
    QString htmlLink = QString(SINGLEGECHIHEAD) + QString::number(QDateTime::currentSecsSinceEpoch()) + QString(SINGLEGECHIMID) + songMid + QString(SINGLEGECHITAIL);

    QNetworkRequest request;
    request.setRawHeader("Referer", "https://y.qq.com/");
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/67.0.3396.99 Safari/537.36");
    request.setUrl(htmlLink);
    QNetworkAccessManager manager;
    QNetworkReply* reply = manager.get(request);

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QString str = reply->readAll();
    reply->deleteLater();

    int start = str.indexOf('{');
    int end = str.lastIndexOf('}');
    QString jsonContent = str.mid(start, end - start + 1);
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonContent.toUtf8(), &parseError);
    QJsonObject root = doc.object();
    QString lyricBase64 = root["lyric"].toString();
    QString transBase64 = root["trans"].toString();
    QByteArray lyricBytes = QByteArray::fromBase64(lyricBase64.toUtf8());
    QByteArray transBytes = QByteArray::fromBase64(transBase64.toUtf8());
    QString lyric = QString::fromUtf8(lyricBytes);
    QString tLyric = QString::fromUtf8(transBytes);

    // lyric
    QStringList lines = lyric.split('\n', Qt::SkipEmptyParts);
    std::vector<std::pair<int, QString>> unsyncLyric;
    for (const QString& line : lines) {
        QString content = line;
        content.remove(tRe_);
        content.trimmed();
        if (!isValidLyric(content))
            continue;
        for (auto it = symbolMap.constBegin(); it != symbolMap.constEnd(); ++it) {
            content.replace(it.key(), it.value());
        }
        content.replace("&apos;", "'");

        QRegularExpressionMatchIterator timeIt = tRe_.globalMatch(line);
        while (timeIt.hasNext()) {
            QRegularExpressionMatch timeMatch = timeIt.next();
            int ms = timeToMilliseconds(timeMatch.captured(1));
            if (ms != -1) {
                unsyncLyric.emplace_back(std::pair<int, QString>(ms, content));
            }
        }
    }
    std::sort(unsyncLyric.begin(), unsyncLyric.end(), [](const auto &p1, const auto &p2){
        return p1.first < p2.first;
    });
    for (const auto &lrc : unsyncLyric) {
        songs_[index].lyric.emplace_back(lrc.second);
        songs_[index].time.emplace_back(lrc.first);
        songs_[index].tLyric.emplace_back("[]");
        songs_[index].rLyric.emplace_back("[]");
    }

    // tLyric
    lines = tLyric.split('\n', Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        QString content = line;
        content.remove(tRe_);
        content.trimmed();
        if (!isValidLyric(content))
            continue;
        for (auto it = symbolMap.constBegin(); it != symbolMap.constEnd(); ++it) {
            content.replace(it.key(), it.value());
        }

        QRegularExpressionMatchIterator timeIt = tRe_.globalMatch(line);
        while (timeIt.hasNext()) {
            QRegularExpressionMatch timeMatch = timeIt.next();
            int ms = timeToMilliseconds(timeMatch.captured(1));
            if (ms != -1) {
                for (int i = 0, j = songs_[index].lyric.size(); i < j; ++i) {
                    int delta = std::abs(ms - songs_[index].time[i]);
                    if (delta <= 1) {
                        songs_[index].tLyric[i] = content;
                    }
                }
            }
        }
    }

    QStringList songList;
    songList << QString::number(index + 1) << songs_[index].lyric[0] << songs_[index].lyric[1] << "0.00" << (tLyric.isEmpty() ? "⛔️" : "❌") << "⛔️";
    songsList_[index] = songList;

    int progress = songsNum_;
    {
        std::lock_guard<std::mutex> lock(mtx);
        for (const QStringList& str : songsList_) {
            if (str.isEmpty())
                progress--;
        }
        emit songsNumberChanged(songsNum_, progress);
    }
}

