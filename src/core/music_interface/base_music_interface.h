#ifndef BASE_MUSIC_INTERFACE_H_
#define BASE_MUSIC_INTERFACE_H_

#include "common/SongTypes.h"

#include <QObject>

class BaseMusicInterface : public QObject {
    Q_OBJECT
public:
    explicit BaseMusicInterface(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~BaseMusicInterface() {}

    // 解析歌单链接,获取歌单id
    virtual bool ParsePlaylistId(const QString &playlist_id) = 0;
    // 解析歌单，导入所有歌曲信息
    virtual void ImportMusic() = 0;

signals:
    // 歌单解析错误信号
    void errorOccurred(const QString &error);
    // 用于显示歌单解析进度
    void songsNumberChanged(int totalNum, int currNum);
    // 歌单解析完成信号
    void taskFinished();

protected:
    std::vector<Song> songs_;
    QString playlist_id_;
};

#endif
