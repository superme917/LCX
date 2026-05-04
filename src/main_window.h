//
// Create By WangYiFan on 2026/04/30
//

#pragma once

#include "ElaWidgetTools/ElaWindow.h"
#include "music_window.h"

class ElaContentDialog;

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
    ElaContentDialog* close_dialog_ = nullptr;
    MusicWindow* music_window_ = nullptr;
};

}  // namespace LCX
