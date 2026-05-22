#include "login.h"
#include "home.h"
#include "ui_login.h"
#include <QThread>
#include <QMessageBox>

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

    // 禁用登录按钮，显示加载状态
    ui->loginBtn->setEnabled(false);
    ui->loginBtn->setText("登录中...");

    // 调用认证服务登录
    AuthService::instance().login(account, password);
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
