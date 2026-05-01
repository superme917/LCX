//
// Create By WangYiFan on 2026/04/30
//

#ifndef WORD_EXPORT_H
#define WORD_EXPORT_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include "core/base_music.h"

class WordExport : public QObject
{
    Q_OBJECT
public:
    explicit WordExport(QObject *parent = nullptr);
    ~WordExport();

    void setExportSongs(const QVector<Song> &songs);
    void setExportPath(const QString &path);

    bool isRunning() const;
    void cancel();

signals:
    void creatWordFinished();
    void exportFinished();
    void exportProgress(int total, int current);
    void exportCanceled();
    void exportError(const QString& error);

public slots:
    void doWork();

private:
    void doExport();
    bool checkCanceled();

    QVector<Song> songs_;
    QString path_;
    QThread* workThread_{nullptr};
    QMutex cancelMutex_;
    bool isCanceled_{false};
    bool isRunning_{false};
};

#endif // WORD_EXPORT_H
