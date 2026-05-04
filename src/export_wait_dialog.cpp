//
// Create By WangYiFan on 2026/05/01
//

#include "export_wait_dialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

namespace LCX {

ExportWaitDialog::ExportWaitDialog(QWidget* parent) : ElaDialog(parent) {
    initDialog();
    setWindowButtonFlags(ElaAppBarType::CloseButtonHint);
    setWindowTitle("正在导出");
    setWindowIcon(QIcon(":/resource/liu.jpg"));
    setFixedSize(400, 200);
    setModal(true);
}

ExportWaitDialog::~ExportWaitDialog() {}

void ExportWaitDialog::initDialog() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 状态文本
    statusText_ = new ElaText("正在导出歌词...", this);
    statusText_->setTextPixelSize(14);

    // 进度条
    progressBar_ = new ElaProgressBar(this);
    progressBar_->setMinimum(0);
    progressBar_->setMaximum(100);
    progressBar_->setTextVisible(true);

    // 进度文本
    QHBoxLayout* progressLayout = new QHBoxLayout();
    progressText_ = new ElaText("0 / 0", this);
    progressText_->setTextPixelSize(12);
    progressLayout->addStretch();
    progressLayout->addWidget(progressText_);

    // 取消按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    cancelButton_ = new ElaPushButton("取消导出", this);
    connect(cancelButton_, &ElaPushButton::clicked, this, &ExportWaitDialog::onCancelClicked);
    buttonLayout->addWidget(cancelButton_);

    mainLayout->addWidget(statusText_);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(progressBar_);
    mainLayout->addLayout(progressLayout);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(buttonLayout);
}

void ExportWaitDialog::setProgress(int total, int current) {
    if (total > 0) {
        progressBar_->setValue(current);
        progressText_->setText(QString("%1 / %2").arg(current).arg(total));
    }
}

void ExportWaitDialog::setMaxTotal(int total) { progressBar_->setMaximum(total); }

void ExportWaitDialog::setStatusText(const QString& text) { statusText_->setText(text); }

void ExportWaitDialog::onCancelClicked() {
    emit cancelRequested();
    reject();
}

}  // namespace LCX