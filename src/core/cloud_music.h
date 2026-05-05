//
// Create By WangYiFan on 2026/05/01
//

#pragma once

#include "core/base_music.h"

#include <QCloudMusicApi/module.h>

namespace LCX::core {

// 网易云音乐解析接口
class CloudMusic : public BaseMusic {
    Q_OBJECT
public:
    // 构造函数
    CloudMusic(QWidget *parent = nullptr);
    // 析构函数
    ~CloudMusic();
    // 解析歌单链接，获取歌词
    virtual void importMusic(const QString &playlist_link) override;

private:
    NeteaseCloudMusicApi *cloud_music_;
};

}  // namespace LCX::core
