//
// Create By WangYiFan on 2026/05/10
//

#pragma once

#include "core/base_music.h"

#include <QNetworkAccessManager>

namespace LCX::core {

class QiShuiMusic : public BaseMusic {
    Q_OBJECT
public:
    // 构造函数
    QiShuiMusic(QWidget *parent = nullptr);
    // 析构函数
    ~QiShuiMusic();
    // 解析歌单链接，获取歌词
    virtual void importMusic(const QString &playlist_link) override;
private slots:
    void onPlaylistReplyFinished();
private:
    void fetchLyric(const QString &id, int song_index);
private:
    QNetworkAccessManager* network_manager_ = nullptr;
    const QString token_ = "07B3B543E4ECD33F1D0F18470DA9F518";
    int process_num_ = 0;
};

}  // namespace LCX::core