//
// Create By WangYiFan on 2026/04/30
//

#include "io/word_export.h"
#include "io/word_engine.h"
#include <QThread>
#include <QApplication>

WordExport::WordExport(QObject *parent)
    : QObject{parent}
{
}

WordExport::~WordExport()
{
    if (workThread_ && workThread_->isRunning()) {
        cancel();
        workThread_->quit();
        workThread_->wait();
    }
}

void WordExport::setExportSongs(const QVector<Song> &songs)
{
    songs_ = songs;
}

void WordExport::setExportPath(const QString &path)
{
    path_ = path;
}

bool WordExport::isRunning() const
{
    return isRunning_;
}

void WordExport::cancel()
{
    QMutexLocker locker(&cancelMutex_);
    isCanceled_ = true;
}

bool WordExport::checkCanceled()
{
    QMutexLocker locker(&cancelMutex_);
    return isCanceled_;
}

void WordExport::doWork()
{
    isRunning_ = true;
    workThread_ = new QThread(this);
    connect(workThread_, &QThread::started, this, &WordExport::doExport);
    connect(workThread_, &QThread::finished, this, [this]() {
        isRunning_ = false;
        workThread_->deleteLater();
        workThread_ = nullptr;
    });
    workThread_->start();
}

void WordExport::doExport()
{
    if (checkCanceled()) {
        emit exportCanceled();
        return;
    }

    QWord word;
    if (!word.createNewWord(path_)){
        emit exportError("创建Word文档失败");
        workThread_->quit();
        return;
    }

    emit creatWordFinished();
    QApplication::processEvents();

    if (checkCanceled()) {
        emit exportCanceled();
        return;
    }

    word.setWordPageView(3);          //页面视图
    word.setFontName(tr("宋体"));
    word.setParagraphAlignment(0);    //下面文字位置

    int currIdx = 0;
    for (int i = 0; i < songs_.size(); ++i) {
        if (checkCanceled()) {
            emit exportCanceled();
            return;
        }

        currIdx++;
        word.setFontBold(true);               //字体加粗
        word.setFontColor("wdColorRed");       //字体颜色
        word.setFontSize(16);
        word.insertText(songs_[i].lyric[0]);
        word.setFontBold(false);
        word.insertMoveDown();
        word.setFontColor("wdColorBlack");    //字体颜色
        word.setFontSize(12);
        for (int j = 1; j < songs_[i].lyric.size(); ++j) {
            word.insertText(songs_[i].lyric[j]);
            word.insertMoveDown();
            if (songs_[i].showTranslate && !songs_[i].tLyric[j].contains('[') && !songs_[i].tLyric[j].contains(']')) {
                word.insertText(songs_[i].tLyric[j]);
                word.insertMoveDown();
            }
            if (songs_[i].showRTranslate && !songs_[i].rLyric[j].contains('[') && !songs_[i].rLyric[j].contains(']')) {
                word.insertText(songs_[i].rLyric[j]);
                word.insertMoveDown();
            }
        }
        word.skidPage();

        emit exportProgress(songs_.size(), currIdx);
        QApplication::processEvents();
    }

    if (checkCanceled()) {
        emit exportCanceled();
        return;
    }

    emit exportFinished();
    QApplication::processEvents(QEventLoop::AllEvents, 1000);
    word.setVisible(true);
    word.saveAs();
}
