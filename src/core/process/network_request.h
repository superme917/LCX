#ifndef NETWORK_REQUEST_H
#define NETWORK_REQUEST_H

#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>

/**
 * @brief 同步阻塞式网络请求模块，内部通过 QEventLoop 阻塞等待
 *
 * 用法:
 *   NetworkRequest nr;
 *   // GET 请求
 *   QByteArray data = nr.get("https://api.example.com/songs");
 *   // POST 请求
 *   QByteArray data = nr.post("https://api.example.com/search", "{\"keyword\":\"test\"}");
 *   // 检查错误
 *   if (nr.lastError() != NetworkRequest::NoError) {
 *       qDebug() << nr.errorString();
 *   }
 *
 * 特点: 事件循环阻塞，调用线程继续处理 Qt 事件，UI 不卡死
 */
class NetworkRequest : public QObject {
    Q_OBJECT

public:
    enum Error { NoError = 0, TimeoutError = 1, NetworkError = 2, AbortedError = 3, UnknownError = 99 };
    Q_ENUM(Error)

    /// @param parent     父对象
    /// @param timeoutMs  超时时间(毫秒)，默认 15000 (15秒)
    explicit NetworkRequest(QObject *parent = nullptr, int timeoutMs = 15000);
    ~NetworkRequest() override;

    // ============================================================
    // 公共接口
    // ============================================================

    /// GET 请求 (同步阻塞)
    QByteArray get(const QString &url);

    /// POST 请求 (同步阻塞)
    /// @param url    请求地址
    /// @param body   请求体 (JSON / Form 等)
    /// @param contentType  Content-Type 头，默认 application/json
    QByteArray post(const QString &url, const QByteArray &body = QByteArray(),
                    const QByteArray &contentType = "application/json");

    /// 设置自定义请求头
    void setHeader(const QByteArray &key, const QByteArray &value);

    /// 设置超时时间(毫秒)
    void setTimeout(int ms);

    /// 获取上一次请求的 HTTP 状态码
    int httpStatusCode() const;

    /// 获取最后一次错误
    Error lastError() const;

    /// 获取错误描述字符串
    QString errorString() const;

private:
    /// 执行请求并阻塞等待
    QByteArray executeRequest(const QString &url, const QByteArray &verb, const QByteArray &body = QByteArray(),
                              const QByteArray &contentType = QByteArray());

    void setError(Error err, const QString &msg);

    QNetworkAccessManager *m_manager;
    QMap<QByteArray, QByteArray> m_headers;
    int m_timeoutMs;
    int m_httpStatusCode;
    Error m_lastError;
    QString m_errorString;
};

#endif  // NETWORK_REQUEST_H
