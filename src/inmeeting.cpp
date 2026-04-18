#include "inmeeting.h"
#include "ui_inmeeting.h"
#include "meeting_engine.h"
#include "meeting_room.h"
#include "meeting_def.h"
#include "local_user.h"
#include "remote_user.h"
#include "videoglwidget.h"

#include <algorithm>
#include <cmath>
#include <QTimer>

extern std::unique_ptr<QWidget> home;
extern std::unique_ptr<QWidget> myprofile;
extern std::unique_ptr<QWidget> joinmeeting;
extern std::unique_ptr<QWidget> inmeeting;
extern std::unique_ptr<QWidget> bookmeeting;
extern std::unique_ptr<QWidget> sharescreen;
extern std::unique_ptr<QWidget> settings;

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

InMeeting::InMeeting(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::InMeeting)
    , meetingEngine_(std::make_unique<MeetingEngine>())
{
    ui->setupUi(this);
    auto localVideoWidget = new VideoGLWidget(this);
    localVideoWidget->setLocal(true);
    videoWidgets_.push_back(localVideoWidget);

    connect(meetingEngine_->room(), &MeetingRoom::sigParticipantJoined,
            this, &InMeeting::onParticipantJoined);
    connect(meetingEngine_->room(), &MeetingRoom::sigTrackSubscribed,
            this, &InMeeting::onTrackSubscribed);

    auto *timer = new QTimer(this);
    timer->setInterval(16);
    connect(timer, &QTimer::timeout, this, &InMeeting::onTimer);
    timer->start();

    meetingEngine_->joinMeeting();
    auto localUser = meetingEngine_->room()->getLocalUser();
    if (localUser) {
        localVideoWidget->setParticipantIdentity(localUser->identity());
        localVideoWidget->setLocal(true);
    }

    // default unmuted and video on
    meetingEngine_->startAudio();
    ui->muteBtn->setText("静音");

    meetingEngine_->startVideo(localVideoWidget->trackSid());
    ui->videoBtn->setText("关闭视频");
}

InMeeting::~InMeeting()
{
    delete ui;
}

void InMeeting::toggleMute()
{
    qInfo() << __FUNCTION__;
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
    qInfo() << __FUNCTION__;
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (button->text() == "开启视频") {
        meetingEngine_->startVideo(videoWidgets_[0]->trackSid());
        button->setText("关闭视频");
    } else {
        meetingEngine_->stopVideo();
        button->setText("开启视频");
    }
}

void InMeeting::toggleRecord()
{
    qInfo() << __FUNCTION__;
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (button) {
        button->setText(button->text() == "录制" ? "停止录制" : "录制");
    }
}

void InMeeting::startShare()
{
    qInfo() << __FUNCTION__;
}

void InMeeting::sendMsg()
{
    qInfo() << __FUNCTION__;
}

void InMeeting::showMember()
{
    qInfo() << __FUNCTION__;
    auto remote_users = meetingEngine_->room()->getRemoteUsers();
}

void InMeeting::inviteUser()
{
    qInfo() << __FUNCTION__;
}

void InMeeting::openChat()
{
    qInfo() << __FUNCTION__;
}

void InMeeting::openApps()
{
    qInfo() << __FUNCTION__;
}

void InMeeting::endMeeting()
{
    qInfo() << __FUNCTION__;
    close();
}

void InMeeting::onParticipantJoined(const QString &participantId, const QString &participantName)
{
    qInfo() << __FUNCTION__
            << "new participant joined, name=" << participantName
            << "id=" << participantId;
}

void InMeeting::onTrackSubscribed(const QString &trackSid, const QString &trackName, 
    const QString &participantIdentity, int trackKind)
{
    qInfo() << __FUNCTION__ 
            << "track subscribed, track_sid=" << trackSid
            << "track_name=" << trackName
            << "participant_identity=" << participantIdentity
            << "track_kind=" << trackKindToMediaTypeString(static_cast<TrackKind>(trackKind));
    switch (static_cast<TrackKind>(trackKind)) {
    case TrackKind::AUDIO:
        break;
    case TrackKind::VIDEO:
    {
        auto videoWidget = new VideoGLWidget(this);
        videoWidget->setParticipantIdentity(participantIdentity.toStdString());
        videoWidget->setTrackSid(trackSid.toStdString());
        videoWidget->setLocal(false);
        videoWidgets_.push_back(videoWidget);
        updateVideoWidgets();
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
    for (int i = 0; i < ui->gridLayout->count(); ++i) {
        QLayoutItem *item = ui->gridLayout->itemAt(i);
        if (!item) {
            continue;
        }

        QWidget *widget = item->widget();
        if (!widget) {
            continue;
        }

        auto *video_widget = qobject_cast<VideoGLWidget *>(widget);
        if (video_widget) {
            video_widget->update();
        }
    }
}

void InMeeting::updateVideoWidgets()
{ 
    qInfo() << __FUNCTION__;
    const int n = videoWidgets_.size();
    if (n <= 0) return;

    // 方案A：固定2列
    // const int cols = 2;

    // 方案B：接近正方形
    const int cols = std::max(1, static_cast<int>(std::ceil(std::sqrt(n))));
    const int rows = (n + cols - 1) / cols;

    // 先从 layout 移除（不 delete 控件）
    while (QLayoutItem *item = ui->gridLayout->takeAt(0)) {
        // item->widget() 仍由父对象管理
        delete item;
    }

    // 按新行列重新放回
    for (int i = 0; i < n; ++i) {
        const int row = i / cols;
        const int col = i % cols;

        ui->gridLayout->addWidget(videoWidgets_[i], row, col);
    }

    // 可选：设置列拉伸，让每列等宽
    for (int c = 0; c < cols; ++c) {
        ui->gridLayout->setColumnStretch(c, 1);
    }
    // 可选：设置行拉伸，让每行等高
    for (int r = 0; r < rows; ++r) {
        ui->gridLayout->setRowStretch(r, 1);
    }
}