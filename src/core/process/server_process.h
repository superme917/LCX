#ifndef SERVER_PROCESS_H
#define SERVER_PROCESS_H

#include <QObject>
#include <QProcess>
#include <QString>

/**
 * @brief 管理第三方二进制服务进程的启动、停止与状态监控
 *
 * 用法:
 *   auto *sp = new ServerProcess(this);
 *   connect(sp, &ServerProcess::started, ...);
 *   connect(sp, &ServerProcess::errorOccurred, ...);
 *   sp->start("/path/to/server", {"--port", "8080"});
 */
class ServerProcess : public QObject {
    Q_OBJECT

public:
    explicit ServerProcess(QObject *parent = nullptr);
    ~ServerProcess() override;

    /// 启动服务器二进制文件
    /// @param binaryPath  二进制文件路径
    /// @return true 表示成功发起启动, false 表示路径无效
    bool start(const QString &binaryPath);

    /// 停止服务器进程
    void stop();

    /// 是否正在运行
    bool isRunning() const;

    /// 获取进程 ID（未运行时返回 -1）
    qint64 processId() const;

signals:
    // 进程成功启动
    void started();

    // 进程已正常退出
    void stopped(int exitCode, QProcess::ExitStatus exitStatus);

    // 进程发生错误
    /// @param errorStr    错误描述
    void errorOccurred(const QString &errorStr);

private slots:
    void onProcessStarted();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessErrorOccurred(QProcess::ProcessError error);

private:
    QProcess *process_;
    QString binaryPath_;
};

#endif  // SERVER_PROCESS_H
