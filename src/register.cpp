#include "register.h"
#include "ui_register.h"
#include "login.h"
#include <QMessageBox>
#include <QRegularExpression>
#include <QCryptographicHash>
#include <QCloseEvent>

Register::Register(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Register)
    , countdownTimer_(nullptr)
    , countdown_(0)
{
    ui->setupUi(this);

    ui->passwordLineEdit->setEchoMode(QLineEdit::Password);
    ui->confirmPasswordLineEdit->setEchoMode(QLineEdit::Password);

    connect(ui->registerBtn, &QPushButton::clicked, this, &Register::onRegister);
    connect(ui->sendCodeBtn, &QPushButton::clicked, this, &Register::onSendCode);
    connect(ui->loginLinkLabel, &QLabel::linkActivated, this, &Register::onLoginLink);

    connect(&AuthService::instance(), &AuthService::sigRegisterSuccess,
            this, &Register::onRegisterSuccess);
    connect(&AuthService::instance(), &AuthService::sigRegisterFailed,
            this, &Register::onRegisterFailed);
}

Register::~Register()
{
    delete ui;
}

void Register::closeEvent(QCloseEvent *event)
{
    if (!registered_) {
        emit ui->loginLinkLabel->linkActivated("#login");
    }
    QWidget::closeEvent(event);
}

void Register::onRegister()
{
    QString account = ui->accountLineEdit->text().trimmed();
    QString password = ui->passwordLineEdit->text();
    QString confirmPassword = ui->confirmPasswordLineEdit->text();
    QString displayName = ui->displayNameLineEdit->text().trimmed();
    QString email = ui->emailLineEdit->text().trimmed();
    QString phone = ui->phoneLineEdit->text().trimmed();
    QString avatarUrl = ui->avatarLineEdit->text().trimmed();

    if (account.isEmpty() || password.isEmpty() || displayName.isEmpty() || email.isEmpty()) {
        QMessageBox::warning(this, "提示", "请填写所有必填项（账号、密码、昵称、邮箱）");
        return;
    }

    if (!validateEmail()) {
        QMessageBox::warning(this, "提示", "请输入正确的邮箱地址");
        return;
    }

    if (!validateAccount()) {
        QMessageBox::warning(this, "提示", "账号只能包含3-20位字母、数字或下划线");
        return;
    }

    if (!validatePassword()) {
        QMessageBox::warning(this, "提示", "密码至少需要8位");
        return;
    }

    if (!passwordsMatch()) {
        QMessageBox::warning(this, "提示", "两次输入的密码不一致");
        return;
    }

    if (!phone.isEmpty() && !validatePhone()) {
        QMessageBox::warning(this, "提示", "请输入正确的手机号");
        return;
    }

    QByteArray hashBytes = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    QString passwordHash = hashBytes.toHex();

    ui->registerBtn->setEnabled(false);
    ui->registerBtn->setText("注册中...");

    AuthService::instance().registerUser(account, passwordHash, displayName, email, phone, avatarUrl);
}

void Register::onSendCode()
{
    QString phone = ui->phoneLineEdit->text().trimmed();

    if (!validatePhone()) {
        QMessageBox::warning(this, "提示", "请先输入正确的手机号");
        return;
    }

    if (!countdownTimer_) {
        countdownTimer_ = new QTimer(this);
        connect(countdownTimer_, &QTimer::timeout, this, &Register::updateSendCodeButton);
    } else {
        countdownTimer_->stop();
    }

    countdown_ = 60;
    ui->sendCodeBtn->setEnabled(false);
    updateSendCodeButton();
    countdownTimer_->start(1000);
}

void Register::updateSendCodeButton()
{
    if (countdown_ > 0) {
        ui->sendCodeBtn->setText(QString("%1秒后重发").arg(countdown_));
        countdown_--;
    } else {
        countdownTimer_->stop();
        ui->sendCodeBtn->setEnabled(true);
        ui->sendCodeBtn->setText("发送验证码");
    }
}

void Register::onLoginLink()
{
    close();
}

void Register::onRegisterSuccess(const UserProfile &profile)
{
    registered_ = true;
    QMessageBox::information(this, "注册成功",
        QString("欢迎加入 MeetEx，%1！").arg(profile.displayName));
    close();
}

void Register::onRegisterFailed(const QString &error)
{
    ui->registerBtn->setEnabled(true);
    ui->registerBtn->setText("注册");
    QMessageBox::critical(this, "注册失败", error);
}

void Register::onCodeSent()
{
    QMessageBox::information(this, "提示", "验证码已发送");
}

void Register::onCodeSendFailed(const QString &error)
{
    QMessageBox::warning(this, "发送失败", error);
    countdownTimer_->stop();
    ui->sendCodeBtn->setEnabled(true);
    ui->sendCodeBtn->setText("发送验证码");
}

bool Register::validateAccount()
{
    QString account = ui->accountLineEdit->text().trimmed();
    QRegularExpression regex("^[a-zA-Z0-9_]{3,20}$");
    return regex.match(account).hasMatch();
}

bool Register::validatePassword()
{
    return ui->passwordLineEdit->text().length() >= 8;
}

bool Register::validateEmail()
{
    QString email = ui->emailLineEdit->text().trimmed();
    QRegularExpression regex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    return regex.match(email).hasMatch();
}

bool Register::validatePhone()
{
    QString phone = ui->phoneLineEdit->text().trimmed();
    if (phone.isEmpty()) return true;
    QRegularExpression regex("^1[3-9]\\d{9}$");
    return regex.match(phone).hasMatch();
}

bool Register::passwordsMatch()
{
    return ui->passwordLineEdit->text() == ui->confirmPasswordLineEdit->text();
}
