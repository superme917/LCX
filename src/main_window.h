#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "ElaWindow.h"

#include <QMainWindow>

class ElaContentDialog;
class MusicWindow;

class MainWindow : public ElaWindow
{
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
#endif // MAIN_WINDOW_H
