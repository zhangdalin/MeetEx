#ifndef REGISTER_H
#define REGISTER_H

#include <QWidget>
#include <QTimer>
#include "services/auth_service.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Register; }
QT_END_NAMESPACE

class Register : public QWidget
{
    Q_OBJECT

public:
    explicit Register(QWidget *parent = nullptr);
    ~Register();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onRegister();
    void onSendCode();
    void onLoginLink();
    void updateSendCodeButton();
    void onRegisterSuccess(const UserProfile &profile);
    void onRegisterFailed(const QString &error);
    void onCodeSent();
    void onCodeSendFailed(const QString &error);

private:
    bool validateAccount();
    bool validatePassword();
    bool validatePhone();
    bool passwordsMatch();

    Ui::Register *ui;
    QTimer *countdownTimer_;
    int countdown_;
    bool registered_ = false;
};

#endif // REGISTER_H
