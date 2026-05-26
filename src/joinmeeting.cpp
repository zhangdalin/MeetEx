#include "joinmeeting.h"
#include "ui_joinmeeting.h"
#include "home.h"

using namespace std;

extern unique_ptr<Home> home;
extern unique_ptr<QWidget> myprofile;
extern unique_ptr<QWidget> joinmeeting;
extern unique_ptr<QWidget> inmeeting;
extern unique_ptr<QWidget> bookmeeting;
extern unique_ptr<QWidget> sharescreen;
extern unique_ptr<QWidget> settings;

JoinMeeting::JoinMeeting(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::JoinMeeting)
    , join_state(false)
{
    ui->setupUi(this);
}

JoinMeeting::~JoinMeeting()
{
    delete ui;
}

void JoinMeeting::onJoinMeeting()
{
    // todo join real action
    join_state = true;
    close();
}

void JoinMeeting::closeEvent(QCloseEvent *event)
{
    if (!inmeeting && join_state) {
        home->onInMeeting();
    } else if (inmeeting && join_state){
        inmeeting->activateWindow();
    }
    emit sigClosing();
    QWidget::closeEvent(event);
}
