#ifndef REGISTER_H
#define REGISTER_H

#include <QWidget>
#include <QTimer>
#include "services/auth_service.h"

QT_BEGIN_NAMESPACE
class QLabel;
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
    void onAvatarUrlChanged(const QString &url);

private:
    bool validateAccount();
    bool validatePassword();
    bool validateEmail();
    bool validatePhone();
    bool passwordsMatch();

    Ui::Register *ui;
    QTimer *countdownTimer_;
    int countdown_;
    bool registered_ = false;
    QLabel *avatarPreviewLabel_ = nullptr;
};

#endif // REGISTER_H
