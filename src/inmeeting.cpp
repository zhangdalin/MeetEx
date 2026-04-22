#include "inmeeting.h"
#include "ui_inmeeting.h"
#include "meeting_engine.h"
#include "meeting_room.h"
#include "meeting_def.h"
#include "local_user.h"
#include "participant.h"
#include "participantwidget.h"

#include <algorithm>
#include <cmath>
#include <unordered_map>
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
        auto *participantContainer = new Participant(this);
        localParticipantId_ = QString::fromStdString(localUser->identity());
        participantContainer->setParticipantName("我");
        participantWidgets_[localParticipantId_] = participantContainer;
    }

    // default unmuted and video on
    meetingEngine_->startAudio();
    ui->muteBtn->setText("静音");

    const auto localIt = participantWidgets_.find(localParticipantId_);
    if (localIt != participantWidgets_.end() && localIt.value()) {
        std::string localVideoSid;
        meetingEngine_->startVideo(localVideoSid);
        localIt.value()->setVideoTrackSid(QString::fromStdString(localVideoSid));
    }
    ui->videoBtn->setText("关闭视频");

    updateVideoWidgets();
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
    const auto localIt = participantWidgets_.find(localParticipantId_);
    Participant *localParticipant = (localIt != participantWidgets_.end()) ? localIt.value() : nullptr;
    ParticipantWidget *localVideoWidget = localParticipant ? localParticipant->getParticipantWidget() : nullptr;
    if (!localVideoWidget) {
        return;
    }
    if (button->text() == "开启视频") {
        std::string localVideoSid;
        meetingEngine_->startVideo(localVideoSid);
        localVideoWidget->setVideoTrackSid(QString::fromStdString(localVideoSid));
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

    auto it = participantWidgets_.find(participantId);
    if (it == participantWidgets_.end()) {
        auto *participantContainer = new Participant(this);
        participantContainer->setParticipantName(participantName);
        participantWidgets_[participantId] = participantContainer;
    } else if (it.value()) {
        it.value()->setParticipantName(participantName);
    }
}

void InMeeting::onTrackSubscribed(const QString &trackSid, const QString &trackName, 
    const QString &participantId, int trackKind)
{
    qInfo() << __FUNCTION__ 
            << "track subscribed, track_sid=" << trackSid
            << "track_name=" << trackName
            << "participant_id=" << participantId
            << "track_kind=" << trackKindToMediaTypeString(static_cast<TrackKind>(trackKind));

    auto it = participantWidgets_.find(participantId);
    Participant *participantContainer = nullptr;
    ParticipantWidget *participantWidget = nullptr;
    if (it == participantWidgets_.end()) {
        participantContainer = new Participant(this);
        participantContainer->setParticipantName(participantId);
        participantWidgets_[participantId] = participantContainer;
        participantWidget = participantContainer->getParticipantWidget();
    } else {
        participantContainer = it.value();
        participantWidget = participantContainer ? participantContainer->getParticipantWidget() : nullptr;
    }

    if (!participantWidget) {
        return;
    }

    switch (static_cast<TrackKind>(trackKind))
    {
    case TrackKind::AUDIO:
        if (!participantWidget->audioTrackSid().isEmpty()) {
            const QString oldAudioSid = participantWidget->audioTrackSid();
            const auto oldIt = audioTrackOwners_.find(oldAudioSid);
            if (oldIt != audioTrackOwners_.end() && oldIt.value() == participantWidget) {
                audioTrackOwners_.erase(oldIt);
            }
        }

        audioTrackOwners_[trackSid] = participantWidget;
        participantWidget->setAudioTrackSid(trackSid);
        break;
    case TrackKind::VIDEO:
        participantWidget->setVideoTrackSid(trackSid);
        break;
    default:
        break;
    }

    updateVideoWidgets();
}

void InMeeting::closeEvent(QCloseEvent *event)
{
    meetingEngine_->endMeeting();
    emit sigClosing();
    QWidget::closeEvent(event);
}

void InMeeting::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    const int leftMargin = 20;
    const int topMargin = 20;
    const int rightMargin = 20;
    const int bottomMargin = 20;
    const int gap = 8;
    const int toolbarHeight = 32;

    const int contentWidth = std::max(0, width() - leftMargin - rightMargin);
    const int toolbarY = std::max(topMargin, height() - bottomMargin - toolbarHeight);
    const int gridHeight = std::max(0, toolbarY - gap - topMargin);

    if (ui->gridLayoutWidget) {
        ui->gridLayoutWidget->setGeometry(leftMargin, topMargin, contentWidth, gridHeight);
    }

    if (ui->layoutWidget) {
        ui->layoutWidget->setGeometry(leftMargin, toolbarY, contentWidth, toolbarHeight);
    }
}

void InMeeting::onTimer()
{
    // 使用缓存的有序 widget 列表，避免每帧遍历 layout
    for (auto *videoWidget : cachedOrderedWidgets_) {
        if (videoWidget) {
            videoWidget->update();
        }
    }

    // 降低音量 UI 更新频率：每 3 帧（~50ms）更新一次，保持 60fps 视频更新
    if (++audioUpdateCounter_ % 3 == 0) {
        updateAudioStatusPanel();
    }
}

void InMeeting::updateAudioStatusPanel()
{
    if (!meetingEngine_) {
        return;
    }

    const AudioLevelInfo local_level = meetingEngine_->localAudioLevel();
    const bool local_speaking = meetingEngine_->isLocalAudioSpeaking();
    const auto localIt = participantWidgets_.find(localParticipantId_);
    if (localIt != participantWidgets_.end() && localIt.value()) {
        localIt.value()->setAudioStatus(local_level.level, local_speaking);
    }

    const auto remote_levels = meetingEngine_->remoteAudioLevels();
    struct ParticipantAudioSnapshot { float level = 0.0f; bool speaking = false; };
    std::unordered_map<ParticipantWidget *, ParticipantAudioSnapshot> participant_audio;

    for (const auto &entry : remote_levels) {
        const QString trackSid = QString::fromStdString(entry.first);
        const auto ownerIt = audioTrackOwners_.find(trackSid);
        if (ownerIt == audioTrackOwners_.end()) {
            continue;
        }

        ParticipantWidget *owner = ownerIt.value().data();
        if (!owner) {
            audioTrackOwners_.erase(ownerIt);
            continue;
        }

        auto &snapshot = participant_audio[owner];
        snapshot.level = std::max(snapshot.level, entry.second.level);
        snapshot.speaking = snapshot.speaking || entry.second.speaking;
    }

    for (auto widgetIt = participantWidgets_.cbegin(); widgetIt != participantWidgets_.cend(); ++widgetIt) {
        const QString &participantId = widgetIt.key();
        auto *participantContainer = widgetIt.value();
        if (!participantContainer || participantId == localParticipantId_) {
            continue;
        }

        ParticipantWidget *participantWidget = participantContainer->getParticipantWidget();
        if (!participantWidget) {
            continue;
        }

        const auto it = participant_audio.find(participantWidget);
        if (it == participant_audio.end()) {
            participantContainer->setAudioStatus(0.0f, false);
        } else {
            participantContainer->setAudioStatus(it->second.level, it->second.speaking);
        }
    }
}

void InMeeting::updateVideoWidgets()
{
    qInfo() << __FUNCTION__;

    // 先从 layout 移除（不 delete 控件，控件仍由父对象管理）
    while (QLayoutItem *item = ui->gridLayout->takeAt(0)) {
        delete item;
    }

    // 本地优先，远端按 id 排序——单次遍历直接收集指针，避免二次 find()
    std::vector<Participant*> orderedParticipants;
    orderedParticipants.reserve(participantWidgets_.size());

    auto localIt = participantWidgets_.find(localParticipantId_);
    if (localIt != participantWidgets_.end() && localIt.value()) {
        orderedParticipants.push_back(localIt.value());
    }

    std::vector<std::pair<QString, Participant*>> remoteEntries;
    remoteEntries.reserve(participantWidgets_.size());
    for (auto widgetIt = participantWidgets_.cbegin(); widgetIt != participantWidgets_.cend(); ++widgetIt) {
        if (widgetIt.key() != localParticipantId_ && widgetIt.value()) {
            remoteEntries.emplace_back(widgetIt.key(), widgetIt.value());
        }
    }
    std::sort(remoteEntries.begin(), remoteEntries.end(),
              [](const auto &a, const auto &b) { return a.first < b.first; });
    for (const auto &entry : remoteEntries) {
        orderedParticipants.push_back(entry.second);
    }

    const int n = static_cast<int>(orderedParticipants.size());
    if (n <= 0) return;

    // 列数基于实际有效 widget 数量
    const int cols = std::max(1, static_cast<int>(std::ceil(std::sqrt(n))));
    const int rows = (n + cols - 1) / cols;

    // 清除旧的拉伸因子，防止布局缩小时遗留残值
    for (int c = 0; c < ui->gridLayout->columnCount(); ++c) {
        ui->gridLayout->setColumnStretch(c, 0);
    }
    for (int r = 0; r < ui->gridLayout->rowCount(); ++r) {
        ui->gridLayout->setRowStretch(r, 0);
    }

    for (int i = 0; i < n; ++i) {
        ui->gridLayout->addWidget(orderedParticipants[i], i / cols, i % cols);
    }

    for (int c = 0; c < cols; ++c) {
        ui->gridLayout->setColumnStretch(c, 1);
    }
    for (int r = 0; r < rows; ++r) {
        ui->gridLayout->setRowStretch(r, 1);
    }

    // 缓存有序 ParticipantWidget 指针供 onTimer 使用
    std::vector<ParticipantWidget*> orderedWidgets;
    orderedWidgets.reserve(orderedParticipants.size());
    for (auto *participant : orderedParticipants) {
        if (participant) {
            ParticipantWidget *widget = participant->getParticipantWidget();
            if (widget) {
                orderedWidgets.push_back(widget);
            }
        }
    }
    cachedOrderedWidgets_.swap(orderedWidgets);
}