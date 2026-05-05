//
// Create By WangYiFan on 2026/04/30
//

#include "music_window.h"

#include <ElaWidgetTools/ElaComboBox.h>
#include <ElaWidgetTools/ElaLineEdit.h>
#include <ElaWidgetTools/ElaMenu.h>
#include <ElaWidgetTools/ElaMessageBar.h>
#include <ElaWidgetTools/ElaProgressBar.h>
#include <ElaWidgetTools/ElaPushButton.h>
#include <ElaWidgetTools/ElaTableView.h>
#include <ElaWidgetTools/ElaText.h>
#include <ElaWidgetTools/ElaTheme.h>
#include <QContextMenuEvent>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>

namespace LCX {

MusicTableViewModel::MusicTableViewModel(QObject* parent) : QAbstractTableModel{parent} {}

MusicTableViewModel::~MusicTableViewModel() {}

int MusicTableViewModel::rowCount(const QModelIndex& parent) const { return data_list_.size(); }

int MusicTableViewModel::columnCount(const QModelIndex& parent) const { return header_.count(); }

QVariant MusicTableViewModel::data(const QModelIndex& index, int role) const {
    if (role == Qt::DisplayRole && index.column() != 0) {
        return data_list_[index.row()][index.column() - 1];
    } else if (role == Qt::DecorationRole && index.column() == 0) {
        return icon_list_[index.row()];
    } else if (role == Qt::DecorationPropertyRole) {
        return Qt::AlignCenter;
    } else if (role == Qt::TextAlignmentRole) {
        return Qt::AlignCenter;
    }
    return QVariant();
}

QVariant MusicTableViewModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return header_[section];
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

void MusicTableViewModel::setHeader() {
    header_ << "预览"
            << "下标"
            << "歌名"
            << "歌手"
            << "时长"
            << "中译"
            << "音译";
}

void MusicTableViewModel::setSongs(const QVector<core::Song>& songs) {
    for (int i = 0; i < songs.size(); ++i) {
        QTime time = QTime::fromMSecsSinceStartOfDay(songs[i].duration);
        QString duration = time.toString("mm:ss");

        QStringList song_list;
        song_list << QString::number(i + 1) << songs[i].name << songs[i].singer << duration
                  << (songs[i].has_tLyric ? disable_emoji : empty) << (songs[i].has_rLyric ? disable_emoji : empty);
        data_list_.push_back(song_list);
    }
}

// TODO:正常使用解析后的链接下载图片，但是图片较大，下载较慢，直接使用指定图片了
void MusicTableViewModel::setIcon(const QVector<core::Song>& songs) {
    for (int i = 0; i < songs.size(); ++i) {
        icon_list_.append(
            QIcon(QPixmap(":/resource/liu.jpg").scaled(38, 38, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    }
}

MusicTableView::MusicTableView(QWidget* parent) : ElaTableView(parent) {
    menu_ = new ElaMenu(this);
    QAction* fanyi = menu_->addElaIconAction(ElaIconType::Xmark, "翻译");
    QAction* yinyi = menu_->addElaIconAction(ElaIconType::Xmark, "音译");
    fanyi->setCheckable(true);
    yinyi->setCheckable(true);
    connect(fanyi, &QAction::triggered, this, [this, fanyi](bool checked) {
        if (checked) {
            fanyi->setProperty("ElaIconType", QChar(ElaIconType::Check));
        } else {
            fanyi->setProperty("ElaIconType", QChar(ElaIconType::Xmark));
        }
        emit translateLyric(checked);
    });
    connect(yinyi, &QAction::triggered, this, [this, yinyi](bool checked) {
        if (checked) {
            yinyi->setProperty("ElaIconType", QChar(ElaIconType::Check));
        } else {
            yinyi->setProperty("ElaIconType", QChar(ElaIconType::Xmark));
        }
        emit rTranslateLyric(checked);
    });
}

void MusicTableView::contextMenuEvent(QContextMenuEvent* event) {
    auto context_idx = indexAt(event->pos());

    if (context_idx.isValid()) {
        menu_->exec(event->globalPos());
    }
    QTableView::contextMenuEvent(event);
}

bool MusicTableViewModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (index.isValid()) {
        beginResetModel();
        data_list_[index.row()][index.column() - 1] = value.toString();
        endResetModel();
        return true;
    }
    return false;
}

MusicWindow::MusicWindow(QWidget* parent) : ElaScrollPage(parent) {
    musicPlatform_ = new ElaComboBox(this);
    musicPlatform_->addItem("网易云音乐");
    musicPlatform_->addItem("QQ音乐");
    musicPlatform_->addItem("酷狗音乐");

    playlistLink_ = new ElaLineEdit(this);
    playlistLink_->setFixedSize(200, 30);

    ElaPushButton* importButton = new ElaPushButton("读取歌单", this);
    ElaPushButton* exportButton = new ElaPushButton("导出歌单", this);

    music_table_view_ = new MusicTableView(this);
    QFont songListHeaderFont = music_table_view_->horizontalHeader()->font();
    songListHeaderFont.setPixelSize(16);
    music_table_view_->setAlternatingRowColors(true);
    music_table_view_->setIconSize(QSize(38, 38));
    music_table_view_->verticalHeader()->setHidden(true);
    music_table_view_->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    music_table_view_->setSelectionBehavior(QAbstractItemView::SelectRows);
    music_table_view_->horizontalHeader()->setMinimumSectionSize(60);
    music_table_view_->horizontalHeader()->setSectionsClickable(true);
    music_table_view_->verticalHeader()->setMinimumSectionSize(46);
    music_table_view_->resizeColumnsToContents();

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
    centerVLayout->addWidget(music_table_view_);
    addCentralWidget(centerWidget_, true, false, 0);

    connect(importButton, &ElaPushButton::clicked, this, &MusicWindow::onImportButtomClicked);
    connect(exportButton, &ElaPushButton::clicked, this, &MusicWindow::onExportButtonClicked);
    connect(music_table_view_, &MusicTableView::translateLyric, this, &MusicWindow::onTranslateLyric);
    connect(music_table_view_, &MusicTableView::rTranslateLyric, this, &MusicWindow::onRTranslateLyric);
    connect(music_table_view_, &ElaTableView::clicked, this, &MusicWindow::onClicked);
}

MusicWindow::~MusicWindow() {}

void MusicWindow::resizeEvent(QResizeEvent* event) {
    music_table_view_->setColumnWidth(0, 60);
    music_table_view_->setColumnWidth(1, 60);
    music_table_view_->setColumnWidth(2, (centerWidget_->width() - 310) / 2);
    music_table_view_->setColumnWidth(3, (centerWidget_->width() - 310) / 2);
    music_table_view_->setColumnWidth(4, 60);
    music_table_view_->setColumnWidth(5, 60);
    music_table_view_->setColumnWidth(6, 60);
}

void MusicWindow::onImportButtomClicked() {
    importProgressText_->setText("0.00%");
    importProgress_->setValue(importProgress_->minimum());

    QString playlistLinkStr = playlistLink_->text();
    if (playlistLinkStr.isEmpty()) {
        ElaMessageBar::warning(ElaMessageBarType::BottomRight, "警告", "还没有填入歌单链接呀😴", 2000, this);
    } else {
        if (musicPlatform_->currentText() == "网易云音乐") {
            music_ = new core::CloudMusic(this);
        } else if (musicPlatform_->currentText() == "QQ音乐") {
            // music_ = new QQMusic(this);
        } else if (musicPlatform_->currentText() == "酷狗音乐") {
            // musicApi_ = new kugouMusic();
        }
        connect(music_, &core::BaseMusic::songsNumberChanged, this, &MusicWindow::onSongsNumberChanged);
        connect(music_, &core::BaseMusic::taskFinished, this, &MusicWindow::ontaskFinished);
        connect(music_, &core::BaseMusic::errorOccurred, this, [this](const QString& error) {
            ElaMessageBar::warning(ElaMessageBarType::BottomRight, "警告", error, 2000, this);
        });
        music_->importMusic(playlistLinkStr);
    }
}

void MusicWindow::onExportButtonClicked() {
    // 检查是否存在解析完成的歌曲
    if (music_ == nullptr || music_->Songs().empty()) {
        ElaMessageBar::warning(ElaMessageBarType::BottomRight, "警告", "还没有导入歌曲哦，先导入歌曲吧！😘", 2000, this);
        return;
    }

    // 选择导出路径
    QString path = QFileDialog::getExistingDirectory(this, "选择导出路径");
    if (path.isEmpty()) {
        return;
    }
    path += "/music.doc";

    // 创建导出等待对话框
    exportWaitDialog_ = new ExportWaitDialog(this);
    connect(exportWaitDialog_, &ExportWaitDialog::cancelRequested, this, &MusicWindow::onExportCanceled);

    // 创建导出对象
    word_export_ = new io::WordExport(this);
    word_export_->setExportSongs(music_->Songs());
    word_export_->setExportPath(path);

    // 连接导出信号
    connect(word_export_, &io::WordExport::exportProgress, this, &MusicWindow::onExportProgress);
    connect(word_export_, &io::WordExport::exportFinished, this, &MusicWindow::onExportFinished);
    connect(word_export_, &io::WordExport::exportCanceled, this, &MusicWindow::onExportCanceled);

    // 显示等待对话框
    exportWaitDialog_->setStatusText("正在导出歌词...");
    exportWaitDialog_->show();
    exportWaitDialog_->activateWindow();

    // 开始导出
    word_export_->doWork();
}

void MusicWindow::onExportProgress(int total, int current) {
    exportWaitDialog_->setMaxTotal(total);
    exportWaitDialog_->setProgress(total, current);
}

void MusicWindow::onExportFinished() {
    if (exportWaitDialog_) {
        exportWaitDialog_->hide();
        exportWaitDialog_->deleteLater();
        exportWaitDialog_ = nullptr;
    }
    ElaMessageBar::success(ElaMessageBarType::PositionPolicy::BottomRight, "Success", "歌单导出成功，我们可太棒了😃", 2000, this);
}

void MusicWindow::onExportCanceled() {
    if (word_export_ && word_export_->isRunning()) {
        word_export_->cancel();
    }
    if (exportWaitDialog_) {
        exportWaitDialog_->hide();
        exportWaitDialog_->deleteLater();
        exportWaitDialog_ = nullptr;
    }
    ElaMessageBar::information(ElaMessageBarType::PositionPolicy::BottomRight, "提示", "已取消导出😐", 2000, this);
}

void MusicWindow::onSongsNumberChanged(int totalNum, int currNum) {
    double value = (double)currNum / totalNum;
    importProgress_->setValue(importProgress_->maximum() * value);
    importProgressText_->setText(QString::number(value * 100, 'f', 2) + "%");
}

void MusicWindow::ontaskFinished() {
    songListModel_ = new MusicTableViewModel(this);
    songListModel_->setHeader();
    songListModel_->setSongs(music_->Songs());
    songListModel_->setIcon(music_->Songs());
    music_table_view_->setModel(songListModel_);
    music_table_view_->setColumnWidth(0, 60);
    music_table_view_->setColumnWidth(1, 60);
    music_table_view_->setColumnWidth(2, (centerWidget_->width() - 310) / 2);
    music_table_view_->setColumnWidth(3, (centerWidget_->width() - 310) / 2);
    music_table_view_->setColumnWidth(4, 60);
    music_table_view_->setColumnWidth(5, 60);
    music_table_view_->setColumnWidth(6, 60);
    // 默认不导出中译和音译
    onTranslateLyric(false);
    onRTranslateLyric(false);
    ElaMessageBar::success(ElaMessageBarType::BottomRight, "Success", "歌单导入成功，我们可太棒了😃", 2000, this);
}

void MusicWindow::onClicked(QModelIndex index) {
    if (index.column() == 5 || index.column() == 6) {
        QString data = index.data().toString();
        if (data == empty) {
            if (index.column() == 5) {
                ElaMessageBar::warning(ElaMessageBarType::BottomRight, "警告", "该歌曲暂不支持中译哦😅", 2000, this);
            } else {
                ElaMessageBar::warning(ElaMessageBarType::BottomRight, "警告", "该歌曲暂不支持中音译哦😅", 2000, this);
            }
            return;
        } else if (data == enable_emoji) {
            music_table_view_->model()->setData(index, disable_emoji);
            if (index.column() == 5) {
                music_->Songs()[index.row()].showTranslate = false;
            } else {
                music_->Songs()[index.row()].showRTranslate = false;
            }
        } else {
            music_table_view_->model()->setData(index, enable_emoji);
            if (index.column() == 5) {
                music_->Songs()[index.row()].showTranslate = true;
            } else {
                music_->Songs()[index.row()].showRTranslate = true;
            }
        }
        music_table_view_->update(index);
    }
}

void MusicWindow::onTranslateLyric(bool check) {
    for (int i = 0; i < music_table_view_->model()->rowCount(); ++i) {
        QModelIndex index = music_table_view_->model()->index(i, 5);
        if (index.data() != empty) {
            music_table_view_->model()->setData(index, QString(check ? enable_emoji : disable_emoji));
            music_->Songs()[i].showTranslate = check;
        }
    }
    music_table_view_->update();
}

void MusicWindow::onRTranslateLyric(bool check) {
    for (int i = 0; i < music_table_view_->model()->rowCount(); ++i) {
        QModelIndex index = music_table_view_->model()->index(i, 6);
        if (index.data() != empty) {
            music_table_view_->model()->setData(index, QString(check ? enable_emoji : disable_emoji));
            music_->Songs()[i].showRTranslate = check;
        }
    }
    music_table_view_->update();
}

}  // namespace LCX