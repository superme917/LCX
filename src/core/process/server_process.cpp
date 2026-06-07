#include "core/process/server_process.h"

#include <QFileInfo>

ServerProcess::ServerProcess(QObject *parent) : QObject(parent), process_(new QProcess(this)) {
    connect(process_, &QProcess::started, this, &ServerProcess::onProcessStarted);
    connect(process_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            &ServerProcess::onProcessFinished);
    connect(process_, &QProcess::errorOccurred, this, &ServerProcess::onProcessErrorOccurred);
}

ServerProcess::~ServerProcess() { stop(); }

bool ServerProcess::start(const QString &binaryPath) {
    if (process_->state() != QProcess::NotRunning) {
        return false;
    }

    QFileInfo fi(binaryPath);
    if (!fi.exists()) {
        emit errorOccurred(QStringLiteral("二进制文件不存在: %1").arg(binaryPath));
        return false;
    }
    if (!fi.isExecutable()) {
        emit errorOccurred(QStringLiteral("二进制文件不可执行: %1").arg(binaryPath));
        return false;
    }

    binaryPath_ = binaryPath;
    process_->setEnvironment(QProcess::systemEnvironment());
    process_->setWorkingDirectory(fi.absolutePath());
    process_->setProcessChannelMode(QProcess::ForwardedErrorChannel);
    process_->start(binaryPath);
    return true;
}

void ServerProcess::stop() {
    if (process_ && process_->state() != QProcess::NotRunning) {
        // 先尝试优雅关闭
        process_->terminate();

        // 等待 3 秒，若未退出则强制杀死
        if (!process_->waitForFinished(3000)) {
            process_->kill();
            process_->waitForFinished(2000);
        }
    }
}

bool ServerProcess::isRunning() const {
    qDebug() << "AAAAAAAAAAAA: " << process_->state();
    return process_->state() == QProcess::Running;
}

qint64 ServerProcess::processId() const {
    if (process_->state() == QProcess::Running) {
        return process_->processId();
    }
    return -1;
}

void ServerProcess::onProcessStarted() { emit started(); }

void ServerProcess::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    emit stopped(exitCode, exitStatus);
}

void ServerProcess::onProcessErrorOccurred(QProcess::ProcessError error) {
    QString msg;
    if (error == QProcess::FailedToStart) {
        msg = QStringLiteral("无法启动进程: %1").arg(binaryPath_);
    } else if (error == QProcess::Timedout) {
        msg = QStringLiteral("进程操作超时: %1").arg(binaryPath_);
    } else if (error == QProcess::WriteError) {
        msg = QStringLiteral("向进程写入数据失败: %1").arg(binaryPath_);
    } else if (error == QProcess::ReadError) {
        msg = QStringLiteral("从进程读取数据失败: %1").arg(binaryPath_);
    } else if (error == QProcess::UnknownError) {
        msg = QStringLiteral("未知进程错误: %1").arg(binaryPath_);
    }

    if (!msg.isEmpty()) {
        emit errorOccurred(msg);
    }
}