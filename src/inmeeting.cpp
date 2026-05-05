#include "inmeeting.h"
#include "ui_inmeeting.h"
#include "meeting_session.h"
#include "meeting_def.h"
#include "remote_user.h"
#include "participant.h"
#include "glwidget.h"

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
    : InMeeting(MeetingSessionCtx::developmentDefaults(), parent)
{
}

InMeeting::InMeeting(const MeetingSessionCtx &context, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::InMeeting)
    , meetingSession_(std::make_unique<MeetingSession>(context, this))
{
    ui->setupUi(this);

    // Connect participant signals
    connect(meetingSession_.get(), &MeetingSession::sigParticipantJoined,
            this, &InMeeting::onParticipantJoined);
    connect(meetingSession_.get(), &MeetingSession::sigParticipantLeft,
            this, &InMeeting::onParticipantLeft);
    connect(meetingSession_.get(), &MeetingSession::sigTrackSubscribed,
            this, &InMeeting::onTrackSubscribed);
    connect(meetingSession_.get(), &MeetingSession::sigTrackUnsubscribed,
            this, &InMeeting::onTrackUnsubscribed);

    // Connect state change signals for UI updates
    connect(meetingSession_.get(), &MeetingSession::sigMicrophoneStateChanged,
            this, &InMeeting::updateButtonStates);
    connect(meetingSession_.get(), &MeetingSession::sigCameraStateChanged,
            this, &InMeeting::updateButtonStates);

    // Timer for video rendering and periodic updates
    auto *timer = new QTimer(this);
    timer->setInterval(16);
    connect(timer, &QTimer::timeout, this, &InMeeting::onTimer);
    timer->start();

    if (meetingSession_->start()) {
        localParticipantId_ = meetingSession_->localParticipantId();
        auto *participant = new Participant(this);
        participant->setParticipantName(context.displayName.trimmed().isEmpty() ? "Me" : context.displayName);
        participants_[localParticipantId_] = participant;
    }

    ui->tabWidget->setVisible(false);
    updateButtonStates();
    updateVideoWidgets();
}

InMeeting::~InMeeting()
{
    delete ui;
}

void InMeeting::toggleMute()
{
    qInfo() << __FUNCTION__;
    if (meetingSession_->microphoneState() == MeetingSessionMediaState::On) {
        meetingSession_->stopAudio();
    } else {
        meetingSession_->startAudio();
    }
}

void InMeeting::toggleVideo()
{
    qInfo() << __FUNCTION__;
    const auto localIt = participants_.find(localParticipantId_);
    Participant *localParticipant = (localIt != participants_.end()) ? localIt.value() : nullptr;
    GLWidget *localGLWidget = localParticipant ? localParticipant->getGLWidget() : nullptr;
    if (!localGLWidget) {
        return;
    }

    if (meetingSession_->cameraState() != MeetingSessionMediaState::On) {
        if (meetingSession_->startVideo()) {
            localGLWidget->setVideoTrackSid(meetingSession_->localVideoTrackSid());
        }
    } else {
        meetingSession_->stopVideo();
        localGLWidget->setVideoTrackSid(QString());
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

void InMeeting::toggleMember()
{
    qInfo() << __FUNCTION__;
    if (ui->tabWidget) {
        const bool visible = ui->tabWidget->isVisible() && ui->tabWidget->currentIndex() == 0;
        ui->tabWidget->setVisible(!visible);
        if (!visible) {
            ui->tabWidget->setCurrentIndex(0);
        }
    }
}

void InMeeting::inviteUser()
{
    qInfo() << __FUNCTION__;
}

void InMeeting::toggleChat()
{
    qInfo() << __FUNCTION__;
    if (ui->tabWidget) {
        const bool visible = ui->tabWidget->isVisible() && ui->tabWidget->currentIndex() == 1;
        ui->tabWidget->setVisible(!visible);
        if (!visible) {
            ui->tabWidget->setCurrentIndex(1);
        }
    }
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

    auto it = participants_.find(participantId);
    if (it == participants_.end()) {
        // New participant - create widget and cache display name
        const QString displayName = meetingSession_->getParticipantDisplayName(participantId, participantName);
        auto *participant = new Participant(this);
        participant->setParticipantName(displayName);
        participants_[participantId] = participant;
    }
    // If participant already exists, we don't need to do anything (tracks will be subscribed later)

    updateAudioStatusPanel();
    updateVideoWidgets();
}

void InMeeting::onParticipantLeft(const QString &participantId, const QString &participantName){
    qInfo() << __FUNCTION__
            << "participant left, name=" << participantName
            << "id=" << participantId;

    // Clean up participant data in meeting session (clears name cache and track mappings)
    meetingSession_->clearParticipantData(participantId);

    // Remove participant widget from UI
    auto it = participants_.find(participantId);
    if (it != participants_.end()) {
        Participant *participant = it.value();
        if (participant) {
            participant->deleteLater();
        }
        participants_.erase(it);
    }

    updateAudioStatusPanel();
    updateVideoWidgets();
}

void InMeeting::onTrackSubscribed(const QString &trackSid, const QString &trackName, 
    const QString &participantId, int trackKind)
{
    qInfo() << __FUNCTION__ 
            << "track subscribed, track_sid=" << trackSid
            << "track_name=" << trackName
            << "participant_id=" << participantId
            << "track_kind=" << trackKindToMediaTypeString(static_cast<TrackKind>(trackKind));

    // Map track to participant in session
    meetingSession_->mapTrackToParticipant(trackSid, participantId);

    auto it = participants_.find(participantId);
    Participant *participant = nullptr;
    GLWidget *glWidget = nullptr;

    if (it == participants_.end()) {
        // Participant widget doesn't exist yet - create it
        const QString displayName = meetingSession_->getParticipantDisplayName(participantId, QString());
        participant = new Participant(this);
        participant->setParticipantName(displayName);
        participants_[participantId] = participant;
        glWidget = participant->getGLWidget();
    } else {
        // Participant widget exists - use it
        participant = it.value();
        glWidget = participant ? participant->getGLWidget() : nullptr;
    }

    if (!glWidget) {
        return;
    }

    // Set the track SID on the GL widget based on track kind
    switch (static_cast<TrackKind>(trackKind)) {
    case TrackKind::AUDIO:
        glWidget->setAudioTrackSid(trackSid);
        break;
    case TrackKind::VIDEO:
        glWidget->setVideoTrackSid(trackSid);
        break;
    default:
        break;
    }

    updateAudioStatusPanel();
    updateVideoWidgets();
}

void InMeeting::onTrackUnsubscribed(const QString &trackSid, const QString &trackName,
    const QString &participantId, int trackKind)
{
    qInfo() << __FUNCTION__
            << "track unsubscribed, track_sid=" << trackSid
            << "track_name=" << trackName
            << "participant_id=" << participantId
            << "track_kind=" << trackKindToMediaTypeString(static_cast<TrackKind>(trackKind));

    // Unmap track from participant in session
    meetingSession_->unmapTrack(trackSid);

    // Get participant and clear the track from its GL widget
    auto participantIt = participants_.find(participantId);
    if (participantIt == participants_.end()) {
        return;  // Participant already removed
    }

    Participant *participant = participantIt.value();
    GLWidget *glWidget = participant ? participant->getGLWidget() : nullptr;

    if (!glWidget) {
        return;
    }

    // Clear the specific track from GL widget
    switch (static_cast<TrackKind>(trackKind)) {
    case TrackKind::AUDIO:
        // Only clear if this is the audio track for this widget
        if (glWidget->audioTrackSid() == trackSid) {
            glWidget->setAudioTrackSid(QString());
        }
        // Reset audio status for participant
        if (participant) {
            participant->setAudioStatus(0.0f, false);
        }
        break;
    case TrackKind::VIDEO:
        // Only clear if this is the video track for this widget
        if (glWidget->videoTrackSid() == trackSid) {
            glWidget->setVideoTrackSid(QString());
        }
        break;
    default:
        break;
    }

    updateAudioStatusPanel();
    updateVideoWidgets();
}

void InMeeting::updateAudioStatusPanel()
{
    if (!meetingSession_) {
        return;
    }

    // Update local participant audio status
    const AudioLevelInfo local_level = meetingSession_->localAudioLevel();
    const bool local_speaking = meetingSession_->isLocalAudioSpeaking();
    const auto localIt = participants_.find(localParticipantId_);
    if (localIt != participants_.end() && localIt.value()) {
        localIt.value()->setAudioStatus(local_level.level, local_speaking);
    }

    // Build participant ID -> audio level map from remote audio levels
    // This is more efficient than looking up participantId for each track
    std::unordered_map<QString, AudioLevelInfo> participantAudioMap;
    const auto remote_levels = meetingSession_->remoteAudioLevels();

    for (const auto &entry : remote_levels) {
        const QString trackSid = QString::fromStdString(entry.first);
        const QString participantId = meetingSession_->getParticipantIdByTrackSid(trackSid);
        
        if (!participantId.isEmpty()) {
            auto &audioInfo = participantAudioMap[participantId];
            // Aggregate audio levels for participant (take max level, any speaking = speaking)
            audioInfo.level = std::max(audioInfo.level, entry.second.level);
            audioInfo.speaking = audioInfo.speaking || entry.second.speaking;
        }
    }

    // Update all remote participants' audio status
    for (auto it = participants_.begin(); it != participants_.end(); ++it) {
        const QString &participantId = it.key();
        auto *participant = it.value();
        
        if (!participant || participantId == localParticipantId_) {
            continue;
        }

        const auto audioIt = participantAudioMap.find(participantId);
        if (audioIt == participantAudioMap.end()) {
            // No audio for this participant
            participant->setAudioStatus(0.0f, false);
        } else {
            participant->setAudioStatus(audioIt->second.level, audioIt->second.speaking);
        }
    }
}

void InMeeting::closeEvent(QCloseEvent *event)
{
    meetingSession_->shutdown();
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

    // 通过 tabWidget 的父 widget（无名内容容器）定位整个内容区域
    if (ui->tabWidget && ui->tabWidget->parentWidget()) {
        ui->tabWidget->parentWidget()->setGeometry(leftMargin, topMargin, contentWidth, gridHeight);
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

void InMeeting::updateButtonStates()
{
    // Update button texts based on MeetingSession state
    if (!meetingSession_) {
        return;
    }

    const auto micState = meetingSession_->microphoneState();
    const auto camState = meetingSession_->cameraState();

    ui->muteBtn->setText(micState == MeetingSessionMediaState::On ? "静音" : "解除静音");
    ui->videoBtn->setText(camState == MeetingSessionMediaState::On ? "关闭视频" : "开启视频");
}

void InMeeting::updateVideoWidgets()
{
    qInfo() << __FUNCTION__;

    // 先从 layout 移除（不 delete 控件，控件仍由父对象管理）
    while (QLayoutItem *item = ui->userGridLayout->takeAt(0)) {
        delete item;
    }

    // 本地优先，远端按 id 排序——单次遍历直接收集指针，避免二次 find()
    std::vector<Participant*> orderedParticipants;
    orderedParticipants.reserve(participants_.size());

    auto localIt = participants_.find(localParticipantId_);
    if (localIt != participants_.end() && localIt.value()) {
        orderedParticipants.push_back(localIt.value());
    }

    std::vector<std::pair<QString, Participant*>> remoteEntries;
    remoteEntries.reserve(participants_.size());
    for (auto widgetIt = participants_.cbegin(); widgetIt != participants_.cend(); ++widgetIt) {
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
    for (int c = 0; c < ui->userGridLayout->columnCount(); ++c) {
        ui->userGridLayout->setColumnStretch(c, 0);
    }
    for (int r = 0; r < ui->userGridLayout->rowCount(); ++r) {
        ui->userGridLayout->setRowStretch(r, 0);
    }

    for (int i = 0; i < n; ++i) {
        ui->userGridLayout->addWidget(orderedParticipants[i], i / cols, i % cols);
    }

    for (int c = 0; c < cols; ++c) {
        ui->userGridLayout->setColumnStretch(c, 1);
    }
    for (int r = 0; r < rows; ++r) {
        ui->userGridLayout->setRowStretch(r, 1);
    }

    // 缓存有序 GLWidget 指针供 onTimer 使用
    std::vector<GLWidget*> orderedWidgets;
    orderedWidgets.reserve(orderedParticipants.size());
    for (auto *participant : orderedParticipants) {
        if (participant) {
            GLWidget *glWidget = participant->getGLWidget();
            if (glWidget) {
                orderedWidgets.push_back(glWidget);
            }
        }
    }
    cachedOrderedWidgets_.swap(orderedWidgets);
}