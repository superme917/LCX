//
// Create By WangYiFan on 2026/04/30
//

#pragma once

// #include "core/qq_music.h"
#include "core/cloud_music.h"
#include "export_wait_dialog.h"
#include "io/word_export.h"

#include <ElaWidgetTools/ElaScrollPage.h>
#include <ElaWidgetTools/ElaTableView.h>
#include <QAbstractTableModel>

class ElaComboBox;
class ElaLineEdit;
class ElaProgressBar;
class ElaMenu;

namespace LCX {

const QString enable_emoji = "✔️";
const QString disable_emoji = "❌";
const QString empty = "";

// 音乐数据表格显示类
class MusicTableViewModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit MusicTableViewModel(QObject *parent = nullptr);
    ~MusicTableViewModel();
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // 设置TableModel标题头
    void setHeader();
    // 设置TableModel数据
    void setSongs(const QVector<core::Song> &songs);
    // 设置TableModel每行数据图标
    void setIcon(const QVector<core::Song> &songs);
    // 修改TableModel数据
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

private:
    QStringList header_;            // 表格标题
    QList<QStringList> data_list_;  // 表格数据
    QList<QIcon> icon_list_;        // 每行数据图标
};

class MusicTableView : public ElaTableView {
    Q_OBJECT
public:
    explicit MusicTableView(QWidget *parent = nullptr);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

Q_SIGNALS:
    Q_SIGNAL void translateLyric(bool check);
    Q_SIGNAL void rTranslateLyric(bool check);

private:
    ElaMenu *menu_;
};

class MusicWindow : public ElaScrollPage {
    Q_OBJECT
public:
    MusicWindow(QWidget *parent = nullptr);
    ~MusicWindow();

protected:
    virtual void resizeEvent(QResizeEvent *event);

private slots:
    // 导入歌单
    void onImportButtomClicked();
    // 导出歌曲为word
    void onExportButtonClicked();
    // 歌词解析进度
    void onSongsNumberChanged(int totalNum, int currNum);
    // 可视化歌单解析结果
    void ontaskFinished();
    // 单击修改某个歌曲是否导出中译和音译
    void onClicked(QModelIndex index);
    // 右击批量修改歌单下的所有歌曲是否导出中译
    void onTranslateLyric(bool check);
    // 右击批量修改歌单下的所有歌曲是否导出音译
    void onRTranslateLyric(bool check);
    // 取消导出
    void onExportCanceled();
    // 导出完成
    void onExportFinished();
    // 更新导出进度
    void onExportProgress(int total, int current);

private:
    ElaComboBox *musicPlatform_{nullptr};
    ElaLineEdit *playlistLink_{nullptr};
    MusicTableView *music_table_view_{nullptr};
    MusicTableViewModel *songListModel_{nullptr};
    ElaProgressBar *importProgress_{nullptr};
    ElaText *importProgressText_{nullptr};
    QWidget *centerWidget_{nullptr};
    ExportWaitDialog *exportWaitDialog_{nullptr};  // word导出进度窗口
    io::WordExport *word_export_{nullptr};         // word导出对象
    core::BaseMusic *music_{nullptr};              // 音乐解析抽象类
};

}  // namespace LCX