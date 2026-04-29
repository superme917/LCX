#include "io/word_export.h"
#include "io/word_engine.h"
#include <QThread>
#include <QApplication>

WordExport::WordExport(QObject *parent)
    : QObject{parent}
{

}

void WordExport::setExportSongs(const QVector<Song> &songs)
{
    songs_ = songs;
}

void WordExport::setExportPath(const QString &path)
{
    path_ = path;
}

void WordExport::doWork()
{
    QWord word;
    if (!word.createNewWord(path_)){
        qDebug() << "failed";
        return;
    }

    emit creatWordFinished();
    QApplication::processEvents();

    word.setWordPageView(3);			//页面视图
    word.setFontName(tr("宋体"));
    word.setParagraphAlignment(0);		//下面文字位置

    int currIdx = 0;
    for (int i = 0; i < songs_.size(); ++i) {
        currIdx++;
        word.setFontBold(true);				//字体加粗
        word.setFontColor("wdColorRed");    //字体颜色
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
        qDebug() << i << ", " << songs_[i].lyric[0];
    }
    emit exportFinished();
    QApplication::processEvents(QEventLoop::AllEvents, 1000);
    word.setVisible(true);
    word.saveAs();
}
