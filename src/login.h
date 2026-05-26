#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include <QTimer>
#include "services/auth_service.h"

namespace Ui {
class Login;
}

// 登录模式枚举
enum class LoginMode {
    AccountLogin,   // 账号密码登录
    PhoneLogin      // 手机验证码登录
};

class Login : public QWidget
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = nullptr);
    ~Login();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onLogin();
    void onLoginSuccess(const UserProfile &profile);
    void onLoginFailed(const QString &error);
    void onRegisterLink();
    void onForgotPassword();
    void onSendCode();
    void onCodeSent();
    void onCodeSendFailed(const QString &error);
    void updateSendCodeButton();
    void checkAutoLogin();

private:
    Ui::Login *ui;

    bool login_state;

    // 验证码发送倒计时
    QTimer *countdownTimer_;
    int countdown_;

    // 手机号验证
    bool validatePhone() const;
};

#endif // LOGIN_H
