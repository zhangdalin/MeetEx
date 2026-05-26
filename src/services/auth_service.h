#ifndef AUTH_SERVICE_H
#define AUTH_SERVICE_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QSettings>
#include <QCryptographicHash>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>

// 用户资料结构
struct UserProfile {
    QString userId;
    QString account;
    QString displayName;
    QString phone;
    QString email;
    QString avatarUrl;

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["userId"] = userId;
        obj["account"] = account;
        obj["displayName"] = displayName;
        obj["phone"] = phone;
        obj["email"] = email;
        obj["avatarUrl"] = avatarUrl;
        return obj;
    }

    void fromJson(const QJsonObject &obj) {
        userId = obj["userId"].toString();
        account = obj["account"].toString();
        displayName = obj["displayName"].toString();
        phone = obj["phone"].toString();
        email = obj["email"].toString();
        avatarUrl = obj["avatarUrl"].toString();
    }
};

// 认证令牌结构
struct AuthToken {
    QString accessToken;
    QString refreshToken;
    QDateTime expiresAt;

    bool isExpired() const {
        return QDateTime::currentDateTime() >= expiresAt;
    }

    bool isValid() const {
        return !accessToken.isEmpty() && !isExpired();
    }
};

class AuthService : public QObject {
    Q_OBJECT
public:
    static AuthService& instance();

    // 登录
    void login(const QString &account, const QString &password);
    void loginWithPhone(const QString &phone, const QString &code);

    // 注册
    void registerUser(const QString &account, const QString &password,
                      const QString &displayName, const QString &email,
                      const QString &phone, const QString &code,
                      const QString &avatarUrl = QString());

    // 发送短信验证码
    void sendSmsCode(const QString &phone, const QString &purpose);

    // 登出
    void logout();

    // 状态查询
    bool isLoggedIn() const { return isLoggedIn_; }
    QString getAccessToken() const;
    UserProfile getCurrentUser() const { return currentUser_; }

    // Token 刷新
    void refreshToken();

signals:
    void sigLoginSuccess(const UserProfile &profile);
    void sigLoginFailed(const QString &error);
    void sigRegisterSuccess(const UserProfile &profile);
    void sigRegisterFailed(const QString &error);
    void sigCodeSent();
    void sigCodeSendFailed(const QString &error);
    void sigTokenRefreshed(const QString &newToken);
    void sigLoggedOut();

private:
    explicit AuthService(QObject *parent = nullptr);
    ~AuthService();

    AuthService(const AuthService&) = delete;
    AuthService& operator=(const AuthService&) = delete;

    // 数据持久化
    void saveAuthData();
    void loadAuthData();
    void clearAuthData();

    // 加密/解密
    QString encryptPassword(const QString &password);
    QString decryptPassword(const QString &encrypted);

    AuthToken currentToken_;
    UserProfile currentUser_;
    bool isLoggedIn_ = false;
    bool isRefreshing_ = false;

    // AUTH-006: Token 预刷新定时器
    QTimer *tokenRefreshTimer_ = nullptr;
    static constexpr int TOKEN_REFRESH_CHECK_INTERVAL_MS = 60000;   // 60秒检查一次
    static constexpr int TOKEN_REFRESH_THRESHOLD_SECS = 300;        // 过期前5分钟刷新

    void setupTokenRefreshTimer();
    void onTokenRefreshTimer();

    static constexpr const char* SETTINGS_GROUP = "auth";
    static constexpr const char* KEY_ACCESS_TOKEN = "accessToken";
    static constexpr const char* KEY_REFRESH_TOKEN = "refreshToken";
    static constexpr const char* KEY_EXPIRES_AT = "expiresAt";
    static constexpr const char* KEY_USER_PROFILE = "userProfile";
    static constexpr const char* KEY_REMEMBER_ME = "rememberMe";
};

#endif // AUTH_SERVICE_H
