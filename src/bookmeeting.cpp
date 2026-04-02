#include "bookmeeting.h"
#include "ui_bookmeeting.h"

using namespace std;

extern unique_ptr<QWidget> home;
extern unique_ptr<QWidget> login;
extern unique_ptr<QWidget> myprofile;
extern unique_ptr<QWidget> joinmeeting;
extern unique_ptr<QWidget> inmeeting;
extern unique_ptr<QWidget> bookmeeting;
extern unique_ptr<QWidget> sharescreen;
extern unique_ptr<QWidget> settings;

BookMeeting::BookMeeting(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BookMeeting)
{
    ui->setupUi(this);
}

BookMeeting::~BookMeeting()
{
    delete ui;
}

void BookMeeting::closeEvent(QCloseEvent *event)
{
    emit sigClosing();
    QWidget::closeEvent(event);
}
