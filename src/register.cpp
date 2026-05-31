#include "register.h"
#include "ui_register.h"
#include "login.h"
#include <QMessageBox>
#include <QRegularExpression>
#include <QCryptographicHash>
#include <QCloseEvent>
#include <QAction>
#include <QToolButton>
#include <QWidgetAction>
#include <QLabel>
#include <QPainter>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

Register::Register(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Register)
    , countdownTimer_(nullptr)
    , countdown_(0)
{
    ui->setupUi(this);

    // 设置输入框左侧图标
    ui->accountLineEdit->addAction(QIcon(":/assets/user.png"), QLineEdit::LeadingPosition);
    ui->passwordLineEdit->addAction(QIcon(":/assets/lock.png"), QLineEdit::LeadingPosition);
    ui->confirmPasswordLineEdit->addAction(QIcon(":/assets/lock.png"), QLineEdit::LeadingPosition);
    ui->displayNameLineEdit->addAction(QIcon(":/assets/user.png"), QLineEdit::LeadingPosition);
    ui->emailLineEdit->addAction(QIcon(":/assets/email.png"), QLineEdit::LeadingPosition);
    ui->phoneLineEdit->addAction(QIcon(":/assets/phone.png"), QLineEdit::LeadingPosition);
    ui->codeLineEdit->addAction(QIcon(":/assets/lock.png"), QLineEdit::LeadingPosition);
    ui->avatarLineEdit->addAction(QIcon(":/assets/user.png"), QLineEdit::LeadingPosition);

    ui->passwordLineEdit->setEchoMode(QLineEdit::Password);
    ui->confirmPasswordLineEdit->setEchoMode(QLineEdit::Password);

    // 密码框右侧添加眼睛图标，按住显示明码，松开恢复隐藏
    QToolButton *passwordEyeBtn = new QToolButton(this);
    passwordEyeBtn->setIcon(QIcon(":/assets/eye.png"));
    passwordEyeBtn->setCursor(Qt::PointingHandCursor);
    passwordEyeBtn->setStyleSheet("QToolButton { border: none; padding: 2px; }");
    QWidgetAction *passwordEyeAction = new QWidgetAction(this);
    passwordEyeAction->setDefaultWidget(passwordEyeBtn);
    ui->passwordLineEdit->addAction(passwordEyeAction, QLineEdit::TrailingPosition);
    connect(passwordEyeBtn, &QToolButton::pressed, this, [this, passwordEyeBtn]() {
        passwordEyeBtn->setIcon(QIcon(":/assets/eye_off.png"));
        ui->passwordLineEdit->setEchoMode(QLineEdit::Normal);
    });
    connect(passwordEyeBtn, &QToolButton::released, this, [this, passwordEyeBtn]() {
        passwordEyeBtn->setIcon(QIcon(":/assets/eye.png"));
        ui->passwordLineEdit->setEchoMode(QLineEdit::Password);
    });

    // 确认密码框添加眼睛图标
    QToolButton *confirmEyeBtn = new QToolButton(this);
    confirmEyeBtn->setIcon(QIcon(":/assets/eye_off.png"));
    confirmEyeBtn->setCursor(Qt::PointingHandCursor);
    confirmEyeBtn->setStyleSheet("QToolButton { border: none; padding: 2px; }");
    QWidgetAction *confirmEyeAction = new QWidgetAction(this);
    confirmEyeAction->setDefaultWidget(confirmEyeBtn);
    ui->confirmPasswordLineEdit->addAction(confirmEyeAction, QLineEdit::TrailingPosition);
    connect(confirmEyeBtn, &QToolButton::pressed, this, [this, confirmEyeBtn]() {
        confirmEyeBtn->setIcon(QIcon(":/assets/eye.png"));
        ui->confirmPasswordLineEdit->setEchoMode(QLineEdit::Normal);
    });
    connect(confirmEyeBtn, &QToolButton::released, this, [this, confirmEyeBtn]() {
        confirmEyeBtn->setIcon(QIcon(":/assets/eye_off.png"));
        ui->confirmPasswordLineEdit->setEchoMode(QLineEdit::Password);
    });

    // 信号连接已在 ui/register.ui 中定义

    connect(&AuthService::instance(), &AuthService::sigRegisterSuccess,
            this, &Register::onRegisterSuccess);
    connect(&AuthService::instance(), &AuthService::sigRegisterFailed,
            this, &Register::onRegisterFailed);
    connect(&AuthService::instance(), &AuthService::sigCodeSent,
            this, &Register::onCodeSent);
    connect(&AuthService::instance(), &AuthService::sigCodeSendFailed,
            this, &Register::onCodeSendFailed);

    // 查找并初始化头像预览标签
    avatarPreviewLabel_ = findChild<QLabel*>("avatarPreviewLabel");
    if (avatarPreviewLabel_) {
        // 加载默认头像并裁剪为圆形
        QPixmap defaultAvatar(":/assets/user.png");
        if (!defaultAvatar.isNull()) {
            QPixmap circular(64, 64);
            circular.fill(Qt::transparent);
            QPainter painter(&circular);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setBrush(QBrush(defaultAvatar.scaled(64, 64, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation)));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(0, 0, 64, 64);
            painter.end();
            avatarPreviewLabel_->setPixmap(circular);
        }

        // 连接头像URL变化信号
        connect(ui->avatarLineEdit, &QLineEdit::textChanged,
                this, &Register::onAvatarUrlChanged);
    }
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
    QString code = ui->codeLineEdit->text().trimmed();
    QString avatarUrl = ui->avatarLineEdit->text().trimmed();

    if (account.isEmpty() || password.isEmpty() || displayName.isEmpty() || email.isEmpty() || phone.isEmpty() || code.isEmpty()) {
        QMessageBox::warning(this, "提示", "请填写所有必填项（账号、密码、昵称、邮箱、手机号、验证码）");
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

    if (!validateEmail()) {
        QMessageBox::warning(this, "提示", "请输入正确的邮箱地址");
        return;
    }

    if (!validatePhone()) {
        QMessageBox::warning(this, "提示", "请输入正确的手机号");
        return;
    }

    QByteArray hashBytes = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    QString passwordHash = hashBytes.toHex();

    ui->registerBtn->setEnabled(false);
    ui->registerBtn->setText("注册中...");

    AuthService::instance().registerUser(account, passwordHash, displayName, email, phone, code, avatarUrl);
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

    // 调用后端 API 发送验证码
    AuthService::instance().sendSmsCode(phone, "register");
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

void Register::onAvatarUrlChanged(const QString &url)
{
    if (!avatarPreviewLabel_) return;

    if (url.isEmpty()) {
        // 恢复默认头像
        QPixmap defaultAvatar(":/assets/user.png");
        if (!defaultAvatar.isNull()) {
            QPixmap circular(64, 64);
            circular.fill(Qt::transparent);
            QPainter painter(&circular);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setBrush(QBrush(defaultAvatar.scaled(64, 64, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation)));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(0, 0, 64, 64);
            painter.end();
            avatarPreviewLabel_->setPixmap(circular);
        }
    } else {
        // 尝试加载网络头像
        QUrl imageUrl(url);
        if (imageUrl.isValid()) {
            QNetworkAccessManager *manager = new QNetworkAccessManager(this);
            connect(manager, &QNetworkAccessManager::finished, this, [this, manager](QNetworkReply *reply) {
                manager->deleteLater();
                if (reply->error() == QNetworkReply::NoError) {
                    QByteArray data = reply->readAll();
                    QPixmap pixmap;
                    if (pixmap.loadFromData(data)) {
                        QPixmap circular(64, 64);
                        circular.fill(Qt::transparent);
                        QPainter painter(&circular);
                        painter.setRenderHint(QPainter::Antialiasing);
                        painter.setBrush(QBrush(pixmap.scaled(64, 64, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation)));
                        painter.setPen(Qt::NoPen);
                        painter.drawEllipse(0, 0, 64, 64);
                        painter.end();
                        avatarPreviewLabel_->setPixmap(circular);
                    }
                }
                reply->deleteLater();
            });
            manager->get(QNetworkRequest(imageUrl));
        }
    }
}
