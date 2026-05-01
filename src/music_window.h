//
// Create By WangYiFan on 2026/04/30
//

#ifndef MUSIC_WINDOW_H_
#define MUSIC_WINDOW_H_

#include <QAbstractTableModel>
#include <ElaWidgetTools/ElaScrollPage.h>
#include <ElaWidgetTools/ElaTableView.h>

class ElaComboBox;
class ElaLineEdit;
class ElaProgressBar;
class BaseMusic;
class WordExport;
class ExportWaitDialog;
class ElaMenu;

class MusicTableViewModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit MusicTableViewModel(QObject* parent = nullptr);
    ~MusicTableViewModel();
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void setHeader();
    void setSongsList(const QVector<QVector<QString>> &songsList);
    void setIconLIst(const QString &pictureLink);
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

private:
    QStringList header_;
    QList<QStringList> dataList_;
    QList<QIcon> iconList_;
};

class MusicTableView : public ElaTableView {
    Q_OBJECT
public:
    explicit MusicTableView(QWidget* parent = nullptr);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;

Q_SIGNALS:
    Q_SIGNAL void translateLyric(bool check);
    Q_SIGNAL void rTranslateLyric(bool check);
private:
    ElaMenu *menu_;
};

class MusicWindow : public ElaScrollPage
{
    Q_OBJECT
public:
    MusicWindow(QWidget *parent = nullptr);
    ~MusicWindow();

protected:
    virtual void resizeEvent(QResizeEvent *event);

private slots:
    void onImportButtomClicked();
    void onExportButtonClicked();
    void onErrorOccurred(QString error);
    void onSongsNumberChanged(int totalNum, int currNum);
    void ontaskFinished();
    void onClicked(QModelIndex index);
    void onTranslateLyric(bool check);
    void onRTranslateLyric(bool check);
    void onExportCanceled();
    void onExportFinished();
    void onExportProgress(int total, int current);
    void onCreatWordFinished();

private:
    ElaComboBox *musicPlatform_{nullptr};
    ElaLineEdit *playlistLink_{nullptr};
    MusicTableView *songList_{nullptr};
    MusicTableViewModel *songListModel_{nullptr};
    ElaProgressBar *importProgress_{nullptr};
    ElaText *importProgressText_{nullptr};
    QWidget *centerWidget_{nullptr};
    ExportWaitDialog *exportWaitDialog_{nullptr};
    WordExport *word_export_{nullptr};

    BaseMusic *music_{nullptr};

    bool showTranslate = false;
    bool showRTranslate = false;
};

#endif
