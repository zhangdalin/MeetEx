#include "login.h"
#include "home.h"
#include "ui_login.h"
#include <QThread>

using namespace std;

unique_ptr<QWidget> home = nullptr;

Login::Login(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Login)
    , login_state(false)
{
    ui->setupUi(this);
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
    // todo login action
    login_state = true;
    close();
}
