#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include "services/auth_service.h"

namespace Ui {
class Login;
}

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

private:
    Ui::Login *ui;

    bool login_state;
};

#endif // LOGIN_H
