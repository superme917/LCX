#ifndef WORD_EXPORT_H
#define WORD_EXPORT_H

#include <QObject>
#include "core/base_music.h"

class WordExport : public QObject
{
    Q_OBJECT
public:
    explicit WordExport(QObject *parent = nullptr);

    void setExportSongs(const QVector<Song> &songs);
    void setExportPath(const QString &path);

signals:
    void creatWordFinished();
    void exportFinished();
    void exportProgress(int total, int current);

public slots:
    void doWork();

private:
    QVector<Song> songs_;
    QString path_;
};

#endif // WORD_EXPORT_H
