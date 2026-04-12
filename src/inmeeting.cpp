#include "inmeeting.h"
#include "ui_inmeeting.h"
#include "meeting_engine.h"
#include "meeting_room.h"

#include <QTimer>

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

    connect(meetingEngine_->room(), &MeetingRoom::sigParticipantJoined,
            this, &InMeeting::onParticipantJoined);
    connect(meetingEngine_->room(), &MeetingRoom::sigTrackSubscribed,
            this, &InMeeting::onTrackSubscribed);

    auto *timer = new QTimer(this);
    timer->setInterval(16);
    connect(timer, &QTimer::timeout, this, &InMeeting::onTimer);
    timer->start();

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

void InMeeting::onParticipantJoined(const QString &participantId, const QString &participantName)
{
    qInfo() << "[InMeeting] new participant joined, name=" << participantName << " id=" << participantId;
}

void InMeeting::onTrackSubscribed(const QString &trackSid, const QString &trackName, const QString &participantIdentity, int trackKind)
{
    qInfo() << "[InMeeting] track subscribed, track_sid=" << trackSid
            << " track_name=" << trackName
            << " participant_identity=" << participantIdentity
            << " track_kind=" << trackKind;
}

void InMeeting::closeEvent(QCloseEvent *event)
{
    meetingEngine_->endMeeting();
    emit sigClosing();
    QWidget::closeEvent(event);
}

void InMeeting::onTimer()
{
    // qInfo() << "onTimer";
}   