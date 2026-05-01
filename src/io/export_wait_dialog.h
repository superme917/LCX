//
// Create By WangYiFan on 2026/05/01
//

#ifndef EXPORT_WAIT_DIALOG_H
#define EXPORT_WAIT_DIALOG_H

#include <QDialog>
#include <ElaWidgetTools/ElaDialog.h>
#include <ElaWidgetTools/ElaProgressBar.h>
#include <ElaWidgetTools/ElaPushButton.h>
#include <ElaWidgetTools/ElaText.h>

class ExportWaitDialog : public ElaDialog
{
    Q_OBJECT
public:
    explicit ExportWaitDialog(QWidget* parent = nullptr);
    ~ExportWaitDialog();

    void setProgress(int total, int current);
    void setMaxTotal(int total);
    void setStatusText(const QString& text);

signals:
    void cancelRequested();

public slots:
    void onCancelClicked();

private:
    void initDialog();

    ElaProgressBar* progressBar_{nullptr};
    ElaText* statusText_{nullptr};
    ElaText* progressText_{nullptr};
    ElaPushButton* cancelButton_{nullptr};
};

#endif // EXPORT_WAIT_DIALOG_H
