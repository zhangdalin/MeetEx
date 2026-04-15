#include "inmeeting.h"
#include "ui_inmeeting.h"
#include "meeting_engine.h"
#include "meeting_room.h"
#include "meeting_def.h"
#include "remote_user.h"
#include "videowidget.h"
#include "videoglwidget.h"

#include <QTimer>

using namespace std;

static const char* trackKindToMediaTypeString(TrackKind track_kind) {
    switch (track_kind) {
    case TrackKind::AUDIO:
        return "audio";
    case TrackKind::VIDEO:
        return "video";
    default:
        return "unknown";
    }
}

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

    meetingEngine_->joinMeeting();
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
    qInfo() << __FUNCTION__ << "toggleMute";
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
    qInfo() << __FUNCTION__ << "toggleVideo";
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
    qInfo() << __FUNCTION__ << "toggleRecord";
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (button) {
        button->setText(button->text() == "录制" ? "停止录制" : "录制");
    }
}

void InMeeting::startShare()
{
    qInfo() << __FUNCTION__ << "startShare";
}

void InMeeting::sendMsg()
{
    qInfo() << __FUNCTION__ << "sendMsg";
}

void InMeeting::showMember()
{
    qInfo() << __FUNCTION__ << "showMember";
    auto remote_users = meetingEngine_->room()->getRemoteUsers();
}

void InMeeting::inviteUser()
{
    qInfo() << __FUNCTION__ << "inviteUser";
}

void InMeeting::openChat()
{
    qInfo() << __FUNCTION__ << "openChat";
}

void InMeeting::openApps()
{
    qInfo() << __FUNCTION__ << "openApps";
}

void InMeeting::endMeeting()
{
    qInfo() << __FUNCTION__ << "endMeeting";
    close();
}

void InMeeting::onParticipantJoined(const QString &participantId, const QString &participantName)
{
    qInfo() << __FUNCTION__ << "new participant joined, name=" << participantName << "id=" << participantId;
    auto videoWidget = new VideoWidget(this);
    ui->gridLayout->addWidget(videoWidget);
}

void InMeeting::onTrackSubscribed(const QString &trackSid, const QString &trackName, const QString &participantIdentity, int trackKind)
{
    qInfo() << __FUNCTION__ << "track subscribed, track_sid=" << trackSid
            << "track_name=" << trackName
            << "participant_identity=" << participantIdentity
            << "track_kind=" << trackKindToMediaTypeString(static_cast<TrackKind>(trackKind));
    switch (static_cast<TrackKind>(trackKind)) {
    case TrackKind::AUDIO:
        break;
    case TrackKind::VIDEO:
    {
        // auto videoWidget = new VideoWidget(this);
        // ui->gridLayout->addWidget(videoWidget);
         auto videoWidget = new VideoGLWidget(this);
         ui->gridLayout->addWidget(videoWidget);
         break;
    }
        break;
    default:
        break;
    }
}

void InMeeting::closeEvent(QCloseEvent *event)
{
    meetingEngine_->endMeeting();
    emit sigClosing();
    QWidget::closeEvent(event);
}

void InMeeting::onTimer()
{
    // qInfo() << __FUNCTION__ << "onTimer";
}   