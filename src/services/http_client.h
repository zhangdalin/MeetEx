#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QEventLoop>
#include <QTimer>
#include <functional>

// AUTH-006: HTTP 请求重试队列项
struct PendingRequest {
    QString url;
    QJsonObject body;
    std::function<void(const QJsonObject&, bool)> callback;
    int retryCount = 0;
};

class HttpClient : public QObject {
    Q_OBJECT
public:
    static HttpClient& instance();

    // 异步 POST 请求
    void post(const QString &url, const QJsonObject &body,
              std::function<void(const QJsonObject &, bool)> callback);

    // 异步 GET 请求
    void get(const QString &url,
             std::function<void(const QJsonObject &, bool)> callback);

    // 同步 POST 请求
    QJsonObject postSync(const QString &url, const QJsonObject &body, bool &ok);

    // 设置请求头
    void setHeader(const QString &key, const QString &value);

    // 清除请求头
    void clearHeaders();

    // AUTH-006: 启用/禁用自动刷新重试
    void setAutoRefreshEnabled(bool enabled);

signals:
    void sigNetworkError(const QString &error);

private:
    explicit HttpClient(QObject *parent = nullptr);
    ~HttpClient();

    HttpClient(const HttpClient&) = delete;
    HttpClient& operator=(const HttpClient&) = delete;

    QNetworkAccessManager *manager_;
    QHash<QString, QString> headers_;

    // AUTH-006: 请求重试队列
    QList<PendingRequest> pendingRequests_;
    bool autoRefreshEnabled_ = false;  // 默认禁用，登录成功后启用
    bool isWaitingForRefresh_ = false;

    void handleReply(QNetworkReply *reply, std::function<void(const QJsonObject &, bool)> callback);
    QJsonObject parseJsonResponse(QNetworkReply *reply, bool &ok);
    QNetworkRequest createRequest(const QString &url);

    // AUTH-006: 401 处理
    void handleUnauthorized(const PendingRequest &request);
    void retryPendingRequests();
    void clearPendingRequests();

private slots:
    void onTokenRefreshed();
    void onTokenRefreshFailed();
};

#endif // HTTP_CLIENT_H
