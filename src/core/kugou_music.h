//
// Create By WangYiFan on 2026/05/10
//

#pragma once

#include "core/base_music.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProcess>

namespace LCX::core {

class KuGouMusic : public BaseMusic {
    Q_OBJECT
public:
    KuGouMusic(QWidget *parent = nullptr);
    ~KuGouMusic();
    virtual void importMusic(const QString &playlist_link) override;

private slots:
    void onPlaylistReplyFinished();

private:
    void fetchPlaylist(const QString &playlist_id);
    void fetchLyric(const QString &hash, int song_index);
    void parseLyric(const QString &content, int song_index);
    void parseTransLyric(const QString &content, int song_index);
private:
    QNetworkAccessManager *network_manager_ = nullptr;
    QProcess *kugou_server_process_ = nullptr;
    int process_num_ = 0;
};

}  // namespace LCX::core