#ifndef QQ_MUSIC_H
#define QQ_MUSIC_H

#include "core/base_music.h"
#include "core/downloader.h"

class QQMusic : public BaseMusic
{
    Q_OBJECT
public:
    QQMusic(QWidget *parent = nullptr);
    ~QQMusic() = default;

    virtual bool checkMusicLink(QString musicLink) override;
    virtual void importMusic() override;

public slots:
    void dowmloadFinished();

private:
    void beginToDownloadSongs();
    void beginToDownloadSongsByQQ(int index);

    downloader *downloader_;
    QStringList songMidList_;
    int curIdx_ = 0;
};

#endif // QQ_MUSIC_H
