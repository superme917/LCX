//
// Create By WangYiFan on 2026/05/01
//

#ifndef CLOUD_MUSIC_H
#define CLOUD_MUSIC_H

#include "core/base_music.h"
#include <QCloudMusicApi/module.h>

class CloudMusic : public BaseMusic
{
    Q_OBJECT
public:
    CloudMusic(QWidget *parent = nullptr);
    ~CloudMusic() = default;

    virtual bool checkMusicLink(QString musicLink) override;
    virtual void importMusic() override;

private:
    NeteaseCloudMusicApi *cloud_music_;
};

#endif // CLOUD_MUSIC_H
