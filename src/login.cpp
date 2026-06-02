#include "login.h"
#include "home.h"
#include "register.h"
#include "ui_login.h"
#include <QThread>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QRegularExpression>
#include <QAction>
#include <QToolButton>
#include <QWidgetAction>

using namespace std;

unique_ptr<Home> home = nullptr;

Login::Login(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Login)
    , login_state(false)
    , countdownTimer_(nullptr)
    , countdown_(0)
{
    ui->setupUi(this);

    // 设置输入框左侧图标
    ui->accountLineEdit->addAction(QIcon(":/assets/user.png"), QLineEdit::LeadingPosition);
    ui->passwordLineEdit->addAction(QIcon(":/assets/lock.png"), QLineEdit::LeadingPosition);
    ui->phoneLineEdit->addAction(QIcon(":/assets/phone.png"), QLineEdit::LeadingPosition);
    ui->codeLineEdit->addAction(QIcon(":/assets/lock.png"), QLineEdit::LeadingPosition);

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

    // 连接认证服务信号
    connect(&AuthService::instance(), &AuthService::sigLoginSuccess,
            this, &Login::onLoginSuccess);
    connect(&AuthService::instance(), &AuthService::sigLoginFailed,
            this, &Login::onLoginFailed);
    connect(&AuthService::instance(), &AuthService::sigCodeSent,
            this, &Login::onCodeSent);
    connect(&AuthService::instance(), &AuthService::sigCodeSendFailed,
            this, &Login::onCodeSendFailed);

    // AUTH-006: 自动登录检查
    checkAutoLogin();
}

Login::~Login()
{
    delete ui;
}

void Login::closeEvent(QCloseEvent *event)
{
    if (login_state) {
        home = make_unique<Home>();
        home->setUserProfile(AuthService::instance().getCurrentUser());
        home->show();
        qInfo() << QThread::currentThread() << __FUNCTION__ << "login logic success";
        QWidget::closeEvent(event);
    }
    else {
        qInfo() << QThread::currentThread() << __FUNCTION__ << "without login, exit!";
        QCoreApplication::exit();
    }
}

void Login::onLogin()
{
    // 根据当前 Tab 选择登录方式
    int currentTab = ui->loginTabWidget->currentIndex();

    if (currentTab == 0) {
        // 账号密码登录
        QString account = ui->accountLineEdit->text().trimmed();
        QString password = ui->passwordLineEdit->text();

        // 验证输入
        if (account.isEmpty() || password.isEmpty()) {
            QMessageBox::warning(this, "提示", "请输入账号和密码");
            return;
        }

        // 对密码进行 SHA256 哈希处理
        QByteArray passwordBytes = password.toUtf8();
        QByteArray hashBytes = QCryptographicHash::hash(passwordBytes, QCryptographicHash::Sha256);
        QString passwordHash = hashBytes.toHex();

        // 禁用登录按钮，显示加载状态
        ui->loginBtn->setEnabled(false);
        ui->loginBtn->setText("登录中...");

        // 调用认证服务登录（发送哈希后的密码）
        AuthService::instance().login(account, passwordHash);
    } else {
        // 手机验证码登录
        QString phone = ui->phoneLineEdit->text().trimmed();
        QString code = ui->codeLineEdit->text().trimmed();

        // 验证输入
        if (phone.isEmpty() || code.isEmpty()) {
            QMessageBox::warning(this, "提示", "请输入手机号和验证码");
            return;
        }

        if (!validatePhone()) {
            QMessageBox::warning(this, "提示", "请输入正确的手机号格式");
            return;
        }

        // 禁用登录按钮，显示加载状态
        ui->loginBtn->setEnabled(false);
        ui->loginBtn->setText("登录中...");

        // 调用手机验证码登录
        AuthService::instance().loginWithPhone(phone, code);
    }
}

void Login::onLoginSuccess(const UserProfile &profile)
{
    login_state = true;
    qInfo() << "Login success:" << profile.displayName;
    close();
}

void Login::onLoginFailed(const QString &error)
{
    // 恢复登录按钮状态
    ui->loginBtn->setEnabled(true);
    ui->loginBtn->setText("登 录");

    QMessageBox::critical(this, "登录失败", error);
}

void Login::onRegisterLink()
{
    // 打开注册窗口
    Register *registerWindow = new Register();
    registerWindow->setAttribute(Qt::WA_DeleteOnClose);
    registerWindow->show();

    // 隐藏登录窗口
    hide();

    // 连接注册窗口关闭信号，重新显示登录窗口
    connect(registerWindow, &Register::destroyed, this, [this]() {
        if (!login_state) {
            show();
        }
    });
}

void Login::onForgotPassword()
{
    // TODO: 实现密码找回功能 (AUTH-004)
    QMessageBox::information(this, "提示", "密码找回功能开发中，请联系管理员");
}

void Login::onSendCode()
{
    QString phone = ui->phoneLineEdit->text().trimmed();

    if (!validatePhone()) {
        QMessageBox::warning(this, "提示", "请输入正确的手机号");
        return;
    }

    // 创建定时器
    if (!countdownTimer_) {
        countdownTimer_ = new QTimer(this);
        connect(countdownTimer_, &QTimer::timeout, this, &Login::updateSendCodeButton);
    } else {
        countdownTimer_->stop();
    }

    // 启动倒计时
    countdown_ = 60;
    ui->sendCodeBtn->setEnabled(false);
    updateSendCodeButton();
    countdownTimer_->start(1000);

    // 调用后端 API 获取验证码
    AuthService::instance().sendSmsCode(phone, "login");
}

void Login::updateSendCodeButton()
{
    if (countdown_ > 0) {
        ui->sendCodeBtn->setText(QString("%1秒后重发").arg(countdown_));
        countdown_--;
    } else {
        countdownTimer_->stop();
        ui->sendCodeBtn->setEnabled(true);
        ui->sendCodeBtn->setText("获取验证码");
    }
}

void Login::onCodeSent()
{
    QMessageBox::information(this, "提示", "验证码已发送，请注意查收短信");
}

void Login::onCodeSendFailed(const QString &error)
{
    QMessageBox::warning(this, "发送失败", error);

    // 停止倒计时，恢复按钮
    if (countdownTimer_) {
        countdownTimer_->stop();
    }
    ui->sendCodeBtn->setEnabled(true);
    ui->sendCodeBtn->setText("获取验证码");
}

bool Login::validatePhone() const
{
    QString phone = ui->phoneLineEdit->text().trimmed();
    QRegularExpression regex("^1[3-9]\\d{9}$");
    return regex.match(phone).hasMatch();
}

void Login::checkAutoLogin()
{
    AuthService &auth = AuthService::instance();

    // 如果已有有效登录，直接进入主页
    if (auth.isLoggedIn() && !auth.getAccessToken().isEmpty()) {
        qInfo() << "Auto-login with existing session";
        login_state = true;
        close();  // 触发 closeEvent 进入主页
    }
}
