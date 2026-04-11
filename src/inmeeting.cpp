#include "inmeeting.h"
#include "ui_inmeeting.h"

#include "glvideowidget.h"
#include "meeting_engine.h"

using namespace std;

extern unique_ptr<QWidget> home;
extern unique_ptr<QWidget> myprofile;
extern unique_ptr<QWidget> joinmeeting;
extern unique_ptr<QWidget> inmeeting;
extern unique_ptr<QWidget> bookmeeting;
extern unique_ptr<QWidget> sharescreen;
extern unique_ptr<QWidget> settings;

InMeeting::InMeeting(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::InMeeting)
    , meetingEngine_(std::make_unique<MeetingEngine>())
{
    ui->setupUi(this);
    videoView_ = new GLVideoWidget(this);
    ui->gridLayout->addWidget(videoView_, 0, 0);
    meetingEngine_->launchMeeting();
    // default unmuted and video on
    meetingEngine_->startAudio();
    ui->muteBtn->setText("静音");
    meetingEngine_->startVideo();
    ui->videoBtn->setText("关闭视频");
}

InMeeting::~InMeeting()
{
    delete ui;
}

void InMeeting::toggleMute()
{
    qInfo() << "toggleMute";
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (button->text() == "静音") {
        meetingEngine_->stopAudio();
        button->setText("解除静音");
    } else {
        meetingEngine_->startAudio();
        button->setText("静音");
    }
}

void InMeeting::toggleVideo()
{
    qInfo() << "toggleVideo";
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (button->text() == "开启视频") {
        meetingEngine_->startVideo();
        button->setText("关闭视频");
    } else {
        meetingEngine_->stopVideo();
        button->setText("开启视频");
    }
}

void InMeeting::toggleRecord()
{
    qInfo() << "toggleRecord";
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (button) {
        button->setText(button->text() == "录制" ? "停止录制" : "录制");
    }
}

void InMeeting::startShare()
{
    qInfo() << "startShare";
}

void InMeeting::sendMsg()
{
    qInfo() << "sendMsg";
}

void InMeeting::showMember()
{
    qInfo() << "showMember";
}

void InMeeting::inviteUser()
{
    qInfo() << "inviteUser";
}

void InMeeting::openChat()
{
    qInfo() << "openChat";
}

void InMeeting::openApps()
{
    qInfo() << "openApps";
}

void InMeeting::endMeeting()
{
    qInfo() << "endMeeting";
    close();
}

void InMeeting::closeEvent(QCloseEvent *event)
{
    meetingEngine_->endMeeting();
    emit sigClosing();
    QWidget::closeEvent(event);
}
