#include "settings.h"
#include "ui_settings.h"
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

Settings::Settings(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Settings)
{
    ui->setupUi(this);
}

Settings::~Settings()
{
    delete ui;
}

void Settings::closeEvent(QCloseEvent *event)
{
    emit sigClosing();
    QWidget::closeEvent(event);
}
