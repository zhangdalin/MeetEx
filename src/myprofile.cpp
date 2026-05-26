#include "myprofile.h"
#include "ui_myprofile.h"
#include "home.h"

using namespace std;

extern unique_ptr<Home> home;
extern unique_ptr<QWidget> login;
extern unique_ptr<QWidget> myprofile;
extern unique_ptr<QWidget> joinmeeting;
extern unique_ptr<QWidget> inmeeting;
extern unique_ptr<QWidget> bookmeeting;
extern unique_ptr<QWidget> sharescreen;
extern unique_ptr<QWidget> settings;

MyProfile::MyProfile(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MyProfile)
{
    ui->setupUi(this);
}

MyProfile::~MyProfile()
{
    delete ui;
}

void MyProfile::closeEvent(QCloseEvent *event)
{
    emit sigClosing();
    QWidget::closeEvent(event);
}
