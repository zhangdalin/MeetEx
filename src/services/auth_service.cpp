#include "auth_service.h"
#include "http_client.h"
#include "meeting_def.h"
#include <QDebug>

AuthService& AuthService::instance() {
    static AuthService instance;
    return instance;
}

AuthService::AuthService(QObject *parent)
    : QObject(parent) {
    loadAuthData();
}

AuthService::~AuthService() {
    if (isLoggedIn_) {
        saveAuthData();
    }
}

void AuthService::login(const QString &account, const QString &password) {
    QJsonObject body;
    body["account"] = account;
    body["password"] = password;

    QString url = QString("%1%2").arg(API_BASE_URL).arg(API_LOGIN_ENDPOINT);

    HttpClient::instance().post(url, body,
        [this](const QJsonObject &response, bool success) {
            if (!success) {
                emit sigLoginFailed("网络请求失败");
                return;
            }

            int code = response["code"].toInt();
            if (code != 200) {
                QString message = response["message"].toString("登录失败");
                emit sigLoginFailed(message);
                return;
            }

            QJsonObject data = response["data"].toObject();

            // 解析 Token
            currentToken_.accessToken = data["accessToken"].toString();
            currentToken_.refreshToken = data["refreshToken"].toString();
            QString expiresIn = data["expiresIn"].toString();
            currentToken_.expiresAt = QDateTime::fromString(expiresIn, Qt::ISODate);
            if (!currentToken_.expiresAt.isValid()) {
                currentToken_.expiresAt = QDateTime::currentDateTime().addSecs(3600);
            }

            // 解析用户资料
            QJsonObject userObj = data["user"].toObject();
            currentUser_.fromJson(userObj);

            isLoggedIn_ = true;
            saveAuthData();

            HttpClient::instance().setHeader("Authorization",
                QString("Bearer %1").arg(currentToken_.accessToken));

            emit sigLoginSuccess(currentUser_);
        });
}

void AuthService::registerUser(const QString &account, const QString &password,
                                const QString &displayName, const QString &email,
                                const QString &phone, const QString &code,
                                const QString &avatarUrl) {
    QJsonObject body;
    body["account"] = account;
    body["password"] = password;
    body["display_name"] = displayName;
    body["email"] = email;
    body["phone"] = phone;
    body["code"] = code;
    if (!avatarUrl.isEmpty()) {
        body["avatar_url"] = avatarUrl;
    }

    QString url = QString("%1%2").arg(API_BASE_URL).arg(API_REGISTER_ENDPOINT);

    HttpClient::instance().post(url, body,
        [this](const QJsonObject &response, bool success) {
            if (!success) {
                emit sigRegisterFailed("网络请求失败");
                return;
            }

            int code = response["code"].toInt();
            if (code != 200) {
                QString message = response["message"].toString("注册失败");
                emit sigRegisterFailed(message);
                return;
            }

            QJsonObject data = response["data"].toObject();

            // 解析 Token
            currentToken_.accessToken = data["accessToken"].toString();
            currentToken_.refreshToken = data["refreshToken"].toString();
            QString expiresIn = data["expiresIn"].toString();
            currentToken_.expiresAt = QDateTime::fromString(expiresIn, Qt::ISODate);
            if (!currentToken_.expiresAt.isValid()) {
                currentToken_.expiresAt = QDateTime::currentDateTime().addSecs(3600);
            }

            // 解析用户资料
            QJsonObject userObj = data["user"].toObject();
            currentUser_.fromJson(userObj);

            isLoggedIn_ = true;
            saveAuthData();

            HttpClient::instance().setHeader("Authorization",
                QString("Bearer %1").arg(currentToken_.accessToken));

            emit sigRegisterSuccess(currentUser_);
        });
}

void AuthService::loginWithPhone(const QString &phone, const QString &code) {
    QJsonObject body;
    body["phone"] = phone;
    body["code"] = code;

    QString url = QString("%1/v1/auth/login/phone").arg(API_BASE_URL);

    HttpClient::instance().post(url, body,
        [this](const QJsonObject &response, bool success) {
            if (!success) {
                emit sigLoginFailed("网络请求失败");
                return;
            }

            int code = response["code"].toInt();
            if (code != 200) {
                QString message = response["message"].toString("登录失败");
                emit sigLoginFailed(message);
                return;
            }

            QJsonObject data = response["data"].toObject();

            currentToken_.accessToken = data["accessToken"].toString();
            currentToken_.refreshToken = data["refreshToken"].toString();
            QString expiresIn = data["expiresIn"].toString();
            currentToken_.expiresAt = QDateTime::fromString(expiresIn, Qt::ISODate);
            if (!currentToken_.expiresAt.isValid()) {
                currentToken_.expiresAt = QDateTime::currentDateTime().addSecs(3600);
            }

            QJsonObject userObj = data["user"].toObject();
            currentUser_.fromJson(userObj);

            isLoggedIn_ = true;
            saveAuthData();

            HttpClient::instance().setHeader("Authorization",
                QString("Bearer %1").arg(currentToken_.accessToken));

            emit sigLoginSuccess(currentUser_);
        });
}

void AuthService::logout() {
    isLoggedIn_ = false;
    clearAuthData();
    HttpClient::instance().clearHeaders();
    emit sigLoggedOut();
}

QString AuthService::getAccessToken() const {
    if (currentToken_.isValid()) {
        return currentToken_.accessToken;
    }
    return QString();
}

void AuthService::refreshToken() {
    if (isRefreshing_ || currentToken_.refreshToken.isEmpty()) {
        return;
    }

    isRefreshing_ = true;

    QJsonObject body;
    body["refreshToken"] = currentToken_.refreshToken;

    QString url = QString("%1%2").arg(API_BASE_URL).arg(API_REFRESH_TOKEN_ENDPOINT);

    HttpClient::instance().post(url, body,
        [this](const QJsonObject &response, bool success) {
            isRefreshing_ = false;

            if (!success) {
                logout();
                return;
            }

            int code = response["code"].toInt();
            if (code != 200) {
                logout();
                return;
            }

            QJsonObject data = response["data"].toObject();

            currentToken_.accessToken = data["accessToken"].toString();
            currentToken_.refreshToken = data["refreshToken"].toString();
            QString expiresIn = data["expiresIn"].toString();
            currentToken_.expiresAt = QDateTime::fromString(expiresIn, Qt::ISODate);
            if (!currentToken_.expiresAt.isValid()) {
                currentToken_.expiresAt = QDateTime::currentDateTime().addSecs(3600);
            }

            saveAuthData();

            HttpClient::instance().setHeader("Authorization",
                QString("Bearer %1").arg(currentToken_.accessToken));

            emit sigTokenRefreshed(currentToken_.accessToken);
        });
}

void AuthService::saveAuthData() {
    QSettings settings;
    settings.beginGroup(SETTINGS_GROUP);
    settings.setValue(KEY_ACCESS_TOKEN, currentToken_.accessToken);
    settings.setValue(KEY_REFRESH_TOKEN, currentToken_.refreshToken);
    settings.setValue(KEY_EXPIRES_AT, currentToken_.expiresAt.toString(Qt::ISODate));
    settings.setValue(KEY_USER_PROFILE, QJsonDocument(currentUser_.toJson()).toJson(QJsonDocument::Compact));
    settings.endGroup();
}

void AuthService::loadAuthData() {
    QSettings settings;
    settings.beginGroup(SETTINGS_GROUP);

    currentToken_.accessToken = settings.value(KEY_ACCESS_TOKEN).toString();
    currentToken_.refreshToken = settings.value(KEY_REFRESH_TOKEN).toString();
    QString expiresAtStr = settings.value(KEY_EXPIRES_AT).toString();
    currentToken_.expiresAt = QDateTime::fromString(expiresAtStr, Qt::ISODate);

    QByteArray userJson = settings.value(KEY_USER_PROFILE).toByteArray();
    if (!userJson.isEmpty()) {
        QJsonDocument doc = QJsonDocument::fromJson(userJson);
        if (!doc.isNull()) {
            currentUser_.fromJson(doc.object());
        }
    }

    settings.endGroup();

    if (!currentToken_.accessToken.isEmpty() && !currentToken_.isExpired()) {
        isLoggedIn_ = true;
        HttpClient::instance().setHeader("Authorization",
            QString("Bearer %1").arg(currentToken_.accessToken));
    }
}

void AuthService::clearAuthData() {
    currentToken_ = AuthToken();
    currentUser_ = UserProfile();

    QSettings settings;
    settings.beginGroup(SETTINGS_GROUP);
    settings.remove("");
    settings.endGroup();
}

QString AuthService::encryptPassword(const QString &password) {
    return QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex();
}

QString AuthService::decryptPassword(const QString &encrypted) {
    return encrypted;
}
