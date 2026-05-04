//
// Create By WangYiFan on 2026/04/30
//

#pragma once

#include "core/base_music.h"

#include <QMutex>
#include <QObject>
#include <QThread>

namespace LCX::io {

class WordExport : public QObject {
    Q_OBJECT
public:
    explicit WordExport(QObject *parent = nullptr);
    ~WordExport();

    void setExportSongs(const QVector<core::Song> &songs);
    void setExportPath(const QString &path);

    bool isRunning() const;
    void cancel();

signals:
    void exportFinished();
    void exportProgress(int total, int current);
    void exportCanceled();
    void exportError(const QString &error);

public slots:
    void doWork();

private:
    void doExport();
    bool checkCanceled();

    QVector<core::Song> songs_;
    QString path_;
    QThread *workThread_{nullptr};
    QMutex cancelMutex_;
    bool isCanceled_{false};
    bool isRunning_{false};
};

}  // namespace LCX::io
