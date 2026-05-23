#include "http_client.h"
#include <QDebug>
#include <QNetworkRequest>
#include <QUrl>
#include <QVariant>
#include <QThread>

HttpClient& HttpClient::instance() {
    static HttpClient instance;
    return instance;
}

HttpClient::HttpClient(QObject *parent)
    : QObject(parent)
    , manager_(new QNetworkAccessManager(this)) {
}

HttpClient::~HttpClient() {
}

void HttpClient::setHeader(const QString &key, const QString &value) {
    headers_[key] = value;
}

void HttpClient::clearHeaders() {
    headers_.clear();
}

QNetworkRequest HttpClient::createRequest(const QString &url) {
    QNetworkRequest request = QNetworkRequest(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));

    // 添加自定义请求头
    for (auto it = headers_.begin(); it != headers_.end(); ++it) {
        request.setRawHeader(it.key().toUtf8(), it.value().toUtf8());
    }

    return request;
}

void HttpClient::post(const QString &url, const QJsonObject &body,
                      std::function<void(const QJsonObject &, bool)> callback) {
    QNetworkRequest request = createRequest(url);
    QJsonDocument doc(body);
    QByteArray data = doc.toJson();

    QNetworkReply *reply = manager_->post(request, data);

    connect(reply, &QNetworkReply::finished, this, [this, reply, callback]() {
        handleReply(reply, callback);
    });
}

void HttpClient::get(const QString &url,
                     std::function<void(const QJsonObject &, bool)> callback) {
    QNetworkRequest request = createRequest(url);
    QNetworkReply *reply = manager_->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, callback]() {
        handleReply(reply, callback);
    });
}

QJsonObject HttpClient::postSync(const QString &url, const QJsonObject &body, bool &ok) {
    QNetworkRequest request = createRequest(url);
    QJsonDocument doc(body);
    QByteArray data = doc.toJson();

    QNetworkReply *reply = manager_->post(request, data);

    // 使用事件循环等待请求完成
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    // 设置 30 秒超时
    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(30000);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start();

    loop.exec();

    if (!timer.isActive()) {
        reply->abort();
        ok = false;
        emit sigNetworkError("请求超时");
        reply->deleteLater();
        return QJsonObject();
    }
    timer.stop();

    return parseJsonResponse(reply, ok);
}

void HttpClient::handleReply(QNetworkReply *reply,
                             std::function<void(const QJsonObject &, bool)> callback) {
    bool ok = false;
    QJsonObject response = parseJsonResponse(reply, ok);
    callback(response, ok);
    reply->deleteLater();
}

QJsonObject HttpClient::parseJsonResponse(QNetworkReply *reply, bool &ok) {
    ok = false;

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    // 尝试解析 JSON，即使有 HTTP 错误也要解析响应体
    if (!doc.isNull()) {
        QJsonObject response = doc.object();

        // 检查 HTTP 状态码
        int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        // 如果是 HTTP 错误（非 2xx），记录错误但返回解析后的 JSON
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << QThread::currentThread() << __FUNCTION__
                << "HTTP error:" << httpStatus << reply->errorString();

            // 如果响应中有错误信息，使用响应中的错误
            if (response.contains("message")) {
                ok = true;  // 成功解析了错误响应
                return response;
            }

            emit sigNetworkError(reply->errorString());
            return QJsonObject();
        }

        // HTTP 成功
        ok = true;
        return response;
    }

    // JSON 解析失败
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << QThread::currentThread() << __FUNCTION__
            << "Network error:" << reply->errorString();
        emit sigNetworkError(reply->errorString());
    } else {
        qWarning() << QThread::currentThread() << __FUNCTION__
            << "Failed to parse JSON response";
        emit sigNetworkError("响应解析失败");
    }

    return QJsonObject();
}
