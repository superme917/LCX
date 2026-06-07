#include "core/process/network_request.h"

#include <QEventLoop>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrl>

NetworkRequest::NetworkRequest(QObject *parent, int timeoutMs)
    : QObject(parent),
      m_manager(new QNetworkAccessManager(this)),
      m_timeoutMs(timeoutMs),
      m_httpStatusCode(0),
      m_lastError(NoError) {}

NetworkRequest::~NetworkRequest() = default;

QByteArray NetworkRequest::get(const QString &url) { return executeRequest(url, "GET"); }

QByteArray NetworkRequest::post(const QString &url, const QByteArray &body, const QByteArray &contentType) {
    return executeRequest(url, "POST", body, contentType);
}

void NetworkRequest::setHeader(const QByteArray &key, const QByteArray &value) { m_headers[key] = value; }

void NetworkRequest::setTimeout(int ms) { m_timeoutMs = ms; }

int NetworkRequest::httpStatusCode() const { return m_httpStatusCode; }

NetworkRequest::Error NetworkRequest::lastError() const { return m_lastError; }

QString NetworkRequest::errorString() const { return m_errorString; }

QByteArray NetworkRequest::executeRequest(const QString &url, const QByteArray &verb, const QByteArray &body,
                                          const QByteArray &contentType) {
    // 重置状态
    m_lastError = NoError;
    m_errorString.clear();
    m_httpStatusCode = 0;

    QUrl qurl(url);
    if (!qurl.isValid()) {
        setError(NetworkError, QStringLiteral("无效的 URL: %1").arg(url));
        return {};
    }

    QNetworkRequest request(qurl);

    // 设置 Content-Type
    if (!contentType.isEmpty()) {
        request.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
    }

    // 设置自定义请求头
    for (auto it = m_headers.cbegin(); it != m_headers.cend(); ++it) {
        request.setRawHeader(it.key(), it.value());
    }

    // 发起请求
    QNetworkReply *reply = nullptr;
    if (verb == "GET") {
        reply = m_manager->get(request);
    } else if (verb == "POST") {
        reply = m_manager->post(request, body);
    } else {
        setError(UnknownError, QStringLiteral("不支持的 HTTP 方法: %1").arg(QString::fromUtf8(verb)));
        return {};
    }

    // ============================================================
    // 核心：用 QEventLoop 阻塞等待，但仍处理 Qt 事件
    // ============================================================
    QEventLoop loop;
    QByteArray resultData;
    bool timedOut = false;

    // 请求完成时退出事件循环
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    // 超时定时器
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &loop, [&]() {
        timedOut = true;
        loop.quit();
    });
    timer.start(m_timeoutMs);

    // 进入事件循环（阻塞当前函数，但 Qt 事件继续处理）
    loop.exec();

    // ============================================================
    // 处理结果
    // ============================================================

    if (timedOut) {
        reply->abort();
        setError(TimeoutError, QStringLiteral("请求超时 (%1 秒): %2").arg(m_timeoutMs / 1000.0).arg(url));
        reply->deleteLater();
        return {};
    }

    // 网络错误
    if (reply->error() != QNetworkReply::NoError) {
        setError(NetworkError, QStringLiteral("网络错误: %1").arg(reply->errorString()));
        reply->deleteLater();
        return {};
    }

    // 获取 HTTP 状态码
    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    m_httpStatusCode = statusCode.isValid() ? statusCode.toInt() : 0;

    // 读取数据
    resultData = reply->readAll();
    reply->deleteLater();

    return resultData;
}

void NetworkRequest::setError(Error err, const QString &msg) {
    m_lastError = err;
    m_errorString = msg;
}
