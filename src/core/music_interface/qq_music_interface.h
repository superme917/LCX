#ifndef QQ_MUSIC_INTERFACE_H
#define QQ_MUSIC_INTERFACE_H

#include "core/music_interface/base_music_interface.h"

class QQMusicInterface : public BaseMusicInterface {
public:
    explicit QQMusicInterface(QObject *parent = nullptr);
    ~QQMusicInterface() override;

    bool ParsePlaylistId(const QString &playlist_id) override;
    void ImportMusic() override;

private:
    QString server_binary_path_;
};

#endif
