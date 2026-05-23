#include "login.h"
#include "home.h"
#include "register.h"
#include "ui_login.h"
#include <QThread>
#include <QMessageBox>
#include <QCryptographicHash>

using namespace std;

unique_ptr<QWidget> home = nullptr;

Login::Login(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Login)
    , login_state(false)
{
    ui->setupUi(this);

    // 连接认证服务信号
    connect(&AuthService::instance(), &AuthService::sigLoginSuccess,
            this, &Login::onLoginSuccess);
    connect(&AuthService::instance(), &AuthService::sigLoginFailed,
            this, &Login::onLoginFailed);
}

Login::~Login()
{
    delete ui;
}

void Login::closeEvent(QCloseEvent *event)
{
    if (login_state) {
        home = make_unique<Home>();
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
    QString account = ui->usernameTextEdit->toPlainText().trimmed();
    QString password = ui->passwordTextEdit->text();

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
    ui->loginBtn->setText("登录");

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
