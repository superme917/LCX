//
// Create By WangYiFan on 2026/04/30
//

#pragma once

#include "ElaWidgetTools/ElaWindow.h"
#include "music_window.h"

#include <QMediaPlayer>

namespace LCX {

class MainWindow : public ElaWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    void initWindow();
    void initEdgeLayout();
    void initContent();
    void initSetting();

private:
    void onCurrentStackIndexChanged();

private:
    MusicWindow* music_window_ = nullptr;
    QMediaPlayer* music_player_ = nullptr;
    QStringList music_files_;
};

}  // namespace LCX
