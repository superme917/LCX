//
// Create By WangYiFan on 2026/04/30
//

#include "music_window.h"

#include <ElaWidgetTools/ElaComboBox.h>
#include <ElaWidgetTools/ElaLineEdit.h>
#include <ElaWidgetTools/ElaPushButton.h>
#include <ElaWidgetTools/ElaTableView.h>
#include <ElaWidgetTools/ElaProgressBar.h>
#include <ElaWidgetTools/ElaText.h>
#include <ElaWidgetTools/ElaTheme.h>
#include <ElaWidgetTools/ElaMessageBar.h>
#include <ElaWidgetTools/ElaMenu.h>
#include <QHeaderView>
#include <QIcon>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QContextMenuEvent>
#include "core/qq_music.h"
#include "core/cloud_music.h"
#include "io/word_export.h"
#include "io/export_wait_dialog.h"

MusicTableViewModel::MusicTableViewModel(QObject* parent)
    : QAbstractTableModel{parent}
{
}

MusicTableViewModel::~MusicTableViewModel()
{
}

int MusicTableViewModel::rowCount(const QModelIndex& parent) const
{
    return dataList_.size();
}

int MusicTableViewModel::columnCount(const QModelIndex& parent) const
{
    return header_.count();
}

QVariant MusicTableViewModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::DisplayRole && index.column() != 0)
    {
        return dataList_[index.row()][index.column() - 1];
    }
    else if (role == Qt::DecorationRole && index.column() == 0)
    {
        return iconList_[index.row()];
    }
    else if (role == Qt::DecorationPropertyRole)
    {
        return Qt::AlignCenter;
    }
    else if (role == Qt::TextAlignmentRole)
    {
        return Qt::AlignCenter;
    }
    return QVariant();
}

QVariant MusicTableViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        return header_[section];
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

void MusicTableViewModel::setHeader()
{
    header_ << "预览" << "下标" << "歌名" << "歌手" << "时长" << "中译" << "音译";
}

void MusicTableViewModel::setSongsList(const QVector<QVector<QString>> &songsList)
{
    dataList_ = songsList;
}

void MusicTableViewModel::setIconLIst(const QString &pictureLink)
{
    for (int i = 0; i < dataList_.size(); ++i) {
        iconList_.append(QIcon(QPixmap(pictureLink).scaled(38, 38, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    }
}

MusicTableView::MusicTableView(QWidget* parent) : ElaTableView(parent) {
    menu_ = new ElaMenu(this);
    QAction* fanyi = menu_->addElaIconAction(ElaIconType::Xmark, "翻译");
    QAction* yinyi = menu_->addElaIconAction(ElaIconType::Xmark, "音译");
    fanyi->setCheckable(true);
    yinyi->setCheckable(true);
    connect(fanyi, &QAction::triggered, this, [this, fanyi](bool checked){
        if (checked) {
            fanyi->setProperty("ElaIconType", QChar(ElaIconType::Check));
        } else {
            fanyi->setProperty("ElaIconType", QChar(ElaIconType::Xmark));
        }
        emit translateLyric(checked);
    });
    connect(yinyi, &QAction::triggered, this, [this, yinyi](bool checked){
        if (checked) {
            yinyi->setProperty("ElaIconType", QChar(ElaIconType::Check));
        } else {
            yinyi->setProperty("ElaIconType", QChar(ElaIconType::Xmark));
        }
        emit rTranslateLyric(checked);
    });
}

void MusicTableView::contextMenuEvent(QContextMenuEvent *event)
{
    auto context_idx = indexAt(event->pos());

    if (context_idx.isValid()) {
        menu_->exec(event->globalPos());
    }
    QTableView::contextMenuEvent(event);
}

bool MusicTableViewModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid()) {
        beginResetModel();
        dataList_[index.row()][index.column() - 1] = value.toString();
        endResetModel();
        return true;
    }
    return false;
}

MusicWindow::MusicWindow(QWidget *parent)
    : ElaScrollPage(parent)
{
    musicPlatform_ = new ElaComboBox(this);
    musicPlatform_->addItem("网易云音乐");
    musicPlatform_->addItem("QQ音乐");
    musicPlatform_->addItem("酷狗音乐");

    playlistLink_ = new ElaLineEdit(this);
    playlistLink_->setFixedSize(200, 30);

    ElaPushButton *importButton = new ElaPushButton("读取歌单", this);
    ElaPushButton *exportButton = new ElaPushButton("导出歌单", this);

    songList_ = new MusicTableView(this);
    QFont songListHeaderFont = songList_->horizontalHeader()->font();
    songListHeaderFont.setPixelSize(16);
    songList_->setAlternatingRowColors(true);
    songList_->setIconSize(QSize(38, 38));
    songList_->verticalHeader()->setHidden(true);
    songList_->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    songList_->setSelectionBehavior(QAbstractItemView::SelectRows);
    songList_->horizontalHeader()->setMinimumSectionSize(60);
    songList_->horizontalHeader()->setSectionsClickable(true);
    songList_->verticalHeader()->setMinimumSectionSize(46);
    songList_->resizeColumnsToContents();

    importProgress_ = new ElaProgressBar(this);
    importProgress_->setMinimum(0);
    importProgress_->setMaximum(100);
    importProgress_->setTextVisible(false);
    importProgressText_ = new ElaText(this);
    importProgressText_->setTextPixelSize(15);
    importProgressText_->setText("0.00%");

    QHBoxLayout* comboBoxLayout = new QHBoxLayout();
    comboBoxLayout->addWidget(musicPlatform_);
    comboBoxLayout->addWidget(playlistLink_);
    comboBoxLayout->addSpacing(10);
    comboBoxLayout->addWidget(importButton);
    comboBoxLayout->addSpacing(10);
    comboBoxLayout->addWidget(exportButton);
    comboBoxLayout->addSpacing(10);
    comboBoxLayout->addWidget(importProgress_);
    comboBoxLayout->addWidget(importProgressText_);
    comboBoxLayout->addStretch();

    centerWidget_ = new QWidget(this);
    centerWidget_->setWindowTitle("MusicTool");
    QVBoxLayout* centerVLayout = new QVBoxLayout(centerWidget_);
    centerVLayout->setContentsMargins(0, 0, 15, 0);
    centerVLayout->addLayout(comboBoxLayout);
    centerVLayout->addSpacing(10);
    centerVLayout->addWidget(songList_);
    addCentralWidget(centerWidget_, true, false, 0);

    connect(importButton, &ElaPushButton::clicked, this, &MusicWindow::onImportButtomClicked);
    connect(exportButton, &ElaPushButton::clicked, this, &MusicWindow::onExportButtonClicked);
    connect(songList_, &MusicTableView::translateLyric, this, &MusicWindow::onTranslateLyric);
    connect(songList_, &MusicTableView::rTranslateLyric, this, &MusicWindow::onRTranslateLyric);
    connect(songList_, &ElaTableView::clicked, this, &MusicWindow::onClicked);
}

MusicWindow::~MusicWindow()
{
}

void MusicWindow::resizeEvent(QResizeEvent *event)
{
    songList_->setColumnWidth(0, 60);
    songList_->setColumnWidth(1, 60);
    songList_->setColumnWidth(2, (centerWidget_->width() - 310) / 2);
    songList_->setColumnWidth(3, (centerWidget_->width() - 310) / 2);
    songList_->setColumnWidth(4, 60);
    songList_->setColumnWidth(5, 60);
    songList_->setColumnWidth(6, 60);
}

void MusicWindow::onImportButtomClicked()
{
    importProgressText_->setText("0.00%");
    importProgress_->setValue(importProgress_->minimum());

    QString playlistLinkStr = playlistLink_->text();
    if (playlistLinkStr.isEmpty()) {
        ElaMessageBar::warning(ElaMessageBarType::BottomRight, "警告", "歌单链接为空!", 2000, this);
    } else {
        if (musicPlatform_->currentText() == "网易云音乐") {
            music_ = new CloudMusic(this);
        } else if (musicPlatform_->currentText() == "QQ音乐") {
            music_ = new QQMusic(this);
        } else if (musicPlatform_->currentText() == "酷狗音乐") {
            // musicApi_ = new kugouMusic();
        }
        connect(music_, &BaseMusic::errorOccurred, this, &MusicWindow::onErrorOccurred);
        connect(music_, &BaseMusic::songsNumberChanged, this, &MusicWindow::onSongsNumberChanged);
        connect(music_, &BaseMusic::taskFinished, this, &MusicWindow::ontaskFinished);
        if (music_->checkMusicLink(playlistLinkStr)) {
            music_->importMusic();
        }
    }
}

void MusicWindow::onExportButtonClicked()
{
    if (music_ == nullptr || music_->getSongsList().empty()) {
        ElaMessageBar::warning(ElaMessageBarType::PositionPolicy::BottomRight, "警告", "暂未导入任何歌曲！", 2000, this);
        return;
    }
    QString path = QFileDialog::getExistingDirectory(this, "选择导出路径");
    if (path.isEmpty()) {
        return;
    }
    path += "/music.doc";

    // 创建等待对话框
    exportWaitDialog_ = new ExportWaitDialog(this);
    connect(exportWaitDialog_, &ExportWaitDialog::cancelRequested, this, &MusicWindow::onExportCanceled);

    // 创建导出对象
    word_export_ = new WordExport(this);
    word_export_->setExportSongs(music_->getSongs());
    word_export_->setExportPath(path);

    // 连接导出信号
    connect(word_export_, &WordExport::creatWordFinished, this, &MusicWindow::onCreatWordFinished);
    connect(word_export_, &WordExport::exportProgress, this, &MusicWindow::onExportProgress);
    connect(word_export_, &WordExport::exportFinished, this, &MusicWindow::onExportFinished);
    connect(word_export_, &WordExport::exportCanceled, this, &MusicWindow::onExportCanceled);

    // 显示等待对话框
    exportWaitDialog_->setStatusText("正在创建Word文档...");
    exportWaitDialog_->show();
    exportWaitDialog_->activateWindow();

    // 开始导出
    word_export_->doWork();
}

void MusicWindow::onCreatWordFinished()
{
    exportWaitDialog_->setStatusText("正在导出歌词...");
}

void MusicWindow::onExportProgress(int total, int current)
{
    exportWaitDialog_->setMaxTotal(total);
    exportWaitDialog_->setProgress(total, current);
}

void MusicWindow::onExportFinished()
{
    if (exportWaitDialog_) {
        exportWaitDialog_->hide();
        exportWaitDialog_->deleteLater();
        exportWaitDialog_ = nullptr;
    }
    ElaMessageBar::success(ElaMessageBarType::PositionPolicy::BottomRight, "Success", "歌单导出成功！", 3000, this);
}

void MusicWindow::onExportCanceled()
{
    if (word_export_ && word_export_->isRunning()) {
        word_export_->cancel();
    }
    if (exportWaitDialog_) {
        exportWaitDialog_->hide();
        exportWaitDialog_->deleteLater();
        exportWaitDialog_ = nullptr;
    }
    ElaMessageBar::warning(ElaMessageBarType::PositionPolicy::BottomRight, "提示", "已取消导出", 2000, this);
}

void MusicWindow::onErrorOccurred(QString error)
{
    ElaMessageBar::warning(ElaMessageBarType::BottomRight, "警告", error, 2000, this);
}

void MusicWindow::onSongsNumberChanged(int totalNum, int currNum)
{
    double value = (double)currNum / totalNum;
    importProgress_->setValue(importProgress_->maximum() * value);
    importProgressText_->setText(QString::number(value * 100, 'f', 2) + "%");
}

void MusicWindow::ontaskFinished()
{
    ElaMessageBar::success(ElaMessageBarType::BottomRight, "Success", "歌单导入成功！", 2000, this);
    songListModel_ = new MusicTableViewModel(this);
    songListModel_->setHeader();
    songListModel_->setSongsList(music_->getSongsList());
    songListModel_->setIconLIst(":/resource/liu.jpg");
    songList_->setModel(songListModel_);
    songList_->setColumnWidth(0, 60);
    songList_->setColumnWidth(1, 60);
    songList_->setColumnWidth(2, (centerWidget_->width() - 310) / 2);
    songList_->setColumnWidth(3, (centerWidget_->width() - 310) / 2);
    songList_->setColumnWidth(4, 60);
    songList_->setColumnWidth(5, 60);
    songList_->setColumnWidth(6, 60);
    onTranslateLyric(showTranslate);
    onRTranslateLyric(showRTranslate);
}

void MusicWindow::onClicked(QModelIndex index)
{
    if (index.column() == 5 || index.column() == 6) {
        QString data = index.data().toString();
        if (data == "⛔️") {
            onErrorOccurred(index.column() == 5 ? "该歌曲不支持中译!" : "该歌曲不支持音译");
            return;
        } else if (data == "✔️") {
            songList_->model()->setData(index, QString("❌"));
            if (index.column() == 5) {
                music_->songs()[index.row()].showTranslate = false;
            } else {
                music_->songs()[index.row()].showRTranslate = false;
            }
        } else {
            songList_->model()->setData(index, QString("✔️"));
            if (index.column() == 5) {
                music_->songs()[index.row()].showTranslate = true;
            } else {
                music_->songs()[index.row()].showRTranslate = true;
            }
        }
        songList_->update(index);
    }
}

void MusicWindow::onTranslateLyric(bool check)
{
    for (int i = 0; i < songList_->model()->rowCount(); ++i) {
        QModelIndex index = songList_->model()->index(i, 5);
        if (index.data() != "⛔️") {
            songList_->model()->setData(index, QString(check ? "✔️" : "❌"));
            music_->songs()[i].showTranslate = check;
        }
    }
    showTranslate = check;
    songList_->update();
}

void MusicWindow::onRTranslateLyric(bool check)
{
    for (int i = 0; i < songList_->model()->rowCount(); ++i) {
        QModelIndex index = songList_->model()->index(i, 6);
        if (index.data() != "⛔️") {
            songList_->model()->setData(index, QString(check ? "✔️" : "❌"));
            music_->songs()[i].showRTranslate = check;
        }
    }
    showRTranslate = check;
    songList_->update();
}