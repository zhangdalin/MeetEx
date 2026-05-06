#include "inmeeting.h"
#include "ui_inmeeting.h"
#include "meeting_session.h"
#include "meeting_def.h"
#include "participantwidget.h"
#include "memberwidget.h"
#include "glwidget.h"

#include <algorithm>
#include <QtGlobal>
#include <QtMath>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>
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
    , meetingSession_(new MeetingSession(context, this))
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
        auto *participant = new ParticipantWidget(this);
        QString localDisplayName = meetingSession_->localParticipantName().trimmed();
        if (localDisplayName.isEmpty()) {
            localDisplayName = context.displayName.trimmed();
        }
        if (localDisplayName.isEmpty()) {
            localDisplayName = localParticipantId_;
        }
        participant->setName(formatMemberDisplayName(localParticipantId_, localDisplayName));
        participantWidgets_[localParticipantId_] = participant;

        // If camera was auto-started during session start, set the video track to local participant
        if (meetingSession_->cameraState() == MeetingSessionMediaState::On) {
            const QString videoTrackSid = meetingSession_->localVideoTrackSid();
            if (!videoTrackSid.isEmpty()) {
                participant->setVideoTrackSid(videoTrackSid);
            }
        }
    }

    ui->tabWidget->setVisible(false);
    ensureMemberListWidget();
    updateMemberList();
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
    GLWidget *localGLWidget = participantWidgetGlWidget(localParticipantId_);
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

    if (!ui->tabWidget) {
        return;
    }

    const bool shouldShowMemberTab = !ui->tabWidget->isVisible() || ui->tabWidget->currentIndex() != 0;
    if (shouldShowMemberTab) {
        ensureMemberListWidget();
        updateMemberList();
        ui->tabWidget->setCurrentIndex(0);
        ui->tabWidget->setVisible(true);
        return;
    }

    ui->tabWidget->setVisible(false);
}

void InMeeting::inviteUser()
{
    qInfo() << __FUNCTION__;
}

void InMeeting::toggleChat()
{
    qInfo() << __FUNCTION__;
    toggleSideTab(1);
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

void InMeeting::onParticipantJoined(const QString &participantId, const QString &name)
{
    qInfo() << __FUNCTION__
            << "new participant joined, name=" << name
            << "id=" << participantId;

    ensureParticipantWidget(participantId, name);

    updateMemberList();
    refreshParticipantViews();
}

void InMeeting::onParticipantLeft(const QString &participantId, const QString &name){
    qInfo() << __FUNCTION__
            << "participant left, name=" << name
            << "id=" << participantId;

    // Clean up participant data in meeting session (clears name cache and track mappings)
    meetingSession_->clearParticipantData(participantId);

    // Remove participant widget from UI
    auto it = participantWidgets_.find(participantId);
    if (it != participantWidgets_.end()) {
        ParticipantWidget *participant = it.value();
        if (participant) {
            participant->deleteLater();
        }
        participantWidgets_.erase(it);
    }

    updateMemberList();
    refreshParticipantViews();
}

void InMeeting::onTrackSubscribed(const QString &trackSid, const QString &trackName, 
    const QString &participantId, int trackKind)
{
    qInfo() << __FUNCTION__ 
            << "track subscribed, track_sid=" << trackSid
            << "track_name=" << trackName
            << "participant_id=" << participantId
            << "track_kind=" << trackKindToMediaTypeString(static_cast<TrackKind>(trackKind));

    ensureParticipantWidget(participantId);
    GLWidget *glWidget = participantWidgetGlWidget(participantId);

    if (!glWidget) {
        return;
    }

    applyTrackToWidget(glWidget, static_cast<TrackKind>(trackKind), trackSid);

    refreshParticipantViews();
}

void InMeeting::onTrackUnsubscribed(const QString &trackSid, const QString &trackName,
    const QString &participantId, int trackKind)
{
    qInfo() << __FUNCTION__
            << "track unsubscribed, track_sid=" << trackSid
            << "track_name=" << trackName
            << "participant_id=" << participantId
            << "track_kind=" << trackKindToMediaTypeString(static_cast<TrackKind>(trackKind));

    ParticipantWidget *participant = participantWidgetById(participantId);
    if (!participant) {
        return;  // ParticipantWidget already removed
    }

    GLWidget *glWidget = participant->getGLWidget();

    if (!glWidget) {
        return;
    }

    clearTrackFromWidget(glWidget, static_cast<TrackKind>(trackKind), trackSid);
    if (static_cast<TrackKind>(trackKind) == TrackKind::AUDIO) {
        participant->setAudioStatus(0.0f, false);
    }

    const bool hasAudioTrack = !participant->audioTrackSid().isEmpty();
    const bool hasVideoTrack = !participant->videoTrackSid().isEmpty();
    if (participantId != localParticipantId_ && !hasAudioTrack && !hasVideoTrack) {
        meetingSession_->clearParticipantData(participantId);

        auto it = participantWidgets_.find(participantId);
        if (it != participantWidgets_.end()) {
            ParticipantWidget *participantToRemove = it.value();
            if (participantToRemove) {
                participantToRemove->deleteLater();
            }
            participantWidgets_.erase(it);
        }

        updateMemberList();
    }

    refreshParticipantViews();
}

void InMeeting::refreshParticipantViews()
{
    updateAudioStatusPanel();
    updateVideoWidgets();
}

void InMeeting::toggleSideTab(int tabIndex)
{
    if (!ui->tabWidget) {
        return;
    }

    const bool visible = ui->tabWidget->isVisible() && ui->tabWidget->currentIndex() == tabIndex;
    ui->tabWidget->setVisible(!visible);
    if (!visible) {
        ui->tabWidget->setCurrentIndex(tabIndex);
    }
}

QString InMeeting::formatMemberDisplayName(const QString &participantId, const QString &baseName) const
{
    const QString trimmedName = baseName.trimmed();
    if (participantId == localParticipantId_) {
        if (trimmedName.endsWith(QStringLiteral("(我)"))) {
            return trimmedName;
        }
        return QStringLiteral("%1 (我)").arg(trimmedName);
    }
    return trimmedName;
}

void InMeeting::ensureMemberListWidget()
{
    if (memberListWidget_) {
        return;
    }

    if (!ui->memberTab) {
        return;
    }

    auto *layout = qobject_cast<QVBoxLayout *>(ui->memberTab->layout());
    if (!layout) {
        layout = new QVBoxLayout(ui->memberTab);
        layout->setContentsMargins(1, 1, 1, 1);
        layout->setSpacing(1);
    }

    auto *listWidget = new QListWidget(ui->memberTab);
    listWidget->setObjectName(QStringLiteral("memberListWidget"));
    layout->addWidget(listWidget);
    memberListWidget_ = listWidget;
}

void InMeeting::updateMemberList()
{
    ensureMemberListWidget();
    if (!memberListWidget_) {
        return;
    }

    memberWidgets_.clear();
    memberListWidget_->clear();

    QStringList participantIds;
    participantIds.reserve(participantWidgets_.size());

    if (!localParticipantId_.isEmpty() && participantWidgets_.contains(localParticipantId_)) {
        participantIds << localParticipantId_;
    }

    QStringList remoteIds;
    for (auto it = participantWidgets_.cbegin(); it != participantWidgets_.cend(); ++it) {
        if (it.key() != localParticipantId_) {
            remoteIds << it.key();
        }
    }
    std::sort(remoteIds.begin(), remoteIds.end());
    participantIds.append(remoteIds);

    for (const QString &participantId : participantIds) {
        ParticipantWidget *participant = participantWidgetById(participantId);
        if (!participant) {
            continue;
        }

        QString displayName = participant->name().trimmed();
        if (displayName.isEmpty()) {
            displayName = meetingSession_->getParticipantDisplayName(participantId, QString());
        }

        auto *item = new QListWidgetItem(memberListWidget_);
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);

        auto *rowWidget = new MemberWidget(memberListWidget_);
        rowWidget->setName(formatMemberDisplayName(participantId, displayName));

        item->setSizeHint(rowWidget->sizeHint());
        memberListWidget_->addItem(item);
        memberListWidget_->setItemWidget(item, rowWidget);
        memberWidgets_.insert(participantId, rowWidget);
    }

    const AudioLevelInfo localAudio = meetingSession_->localAudioLevel();
    const bool localSpeaking = meetingSession_->isLocalAudioSpeaking();
    const QHash<QString, AudioLevelInfo> remoteAudioMap = buildRemoteParticipantAudioMap();
    updateMemberAudioBars(localAudio, localSpeaking, remoteAudioMap);
}

QHash<QString, AudioLevelInfo> InMeeting::buildRemoteParticipantAudioMap() const
{
    QHash<QString, AudioLevelInfo> participantAudioMap;
    const auto remoteLevels = meetingSession_->remoteAudioLevels();

    for (auto it = remoteLevels.begin(); it != remoteLevels.end(); ++it) {
        const QString participantId = meetingSession_->getParticipantIdByTrackSid(it.key());
        if (participantId.isEmpty()) {
            continue;
        }

        const AudioLevelInfo &audioInfo = it.value();
        auto &aggAudioInfo = participantAudioMap[participantId];
        aggAudioInfo.level = qMax(aggAudioInfo.level, audioInfo.level);
        aggAudioInfo.speaking = aggAudioInfo.speaking || audioInfo.speaking;
    }

    return participantAudioMap;
}

void InMeeting::updateMemberAudioBars(const AudioLevelInfo &localAudio, bool localSpeaking,
    const QHash<QString, AudioLevelInfo> &remoteAudioMap)
{
    if (memberWidgets_.isEmpty()) {
        return;
    }

    for (auto it = memberWidgets_.begin(); it != memberWidgets_.end(); ++it) {
        MemberWidget *member = it.value();
        if (!member) {
            continue;
        }

        AudioLevelInfo info;
        if (it.key() == localParticipantId_) {
            info = localAudio;
            info.speaking = localSpeaking;
        } else {
            info = remoteAudioMap.value(it.key());
        }

        member->setAudioStatus(info.level, info.speaking);
    }
}

ParticipantWidget *InMeeting::participantWidgetById(const QString &participantId) const
{
    const auto it = participantWidgets_.find(participantId);
    return it != participantWidgets_.end() ? it.value() : nullptr;
}

ParticipantWidget *InMeeting::ensureParticipantWidget(const QString &participantId,
    const QString &participantNameHint)
{
    if (ParticipantWidget *existing = participantWidgetById(participantId)) {
        const QString displayName = meetingSession_->getParticipantDisplayName(participantId, participantNameHint);
        if (!displayName.trimmed().isEmpty()) {
            existing->setName(formatMemberDisplayName(participantId, displayName));
        }
        return existing;
    }

    const QString displayName = meetingSession_->getParticipantDisplayName(participantId, participantNameHint);
    auto *participant = new ParticipantWidget(this);
    participant->setName(formatMemberDisplayName(participantId, displayName));
    participantWidgets_[participantId] = participant;
    return participant;
}

GLWidget *InMeeting::participantWidgetGlWidget(const QString &participantId) const
{
    ParticipantWidget *participant = participantWidgetById(participantId);
    return participant ? participant->getGLWidget() : nullptr;
}

void InMeeting::applyTrackToWidget(GLWidget *glWidget, TrackKind trackKind, const QString &trackSid) const
{
    if (!glWidget) {
        return;
    }

    switch (trackKind) {
    case TrackKind::AUDIO:
        glWidget->setAudioTrackSid(trackSid);
        break;
    case TrackKind::VIDEO:
        glWidget->setVideoTrackSid(trackSid);
        break;
    default:
        break;
    }
}

void InMeeting::clearTrackFromWidget(GLWidget *glWidget, TrackKind trackKind, const QString &trackSid) const
{
    if (!glWidget) {
        return;
    }

    switch (trackKind) {
    case TrackKind::AUDIO:
        if (glWidget->audioTrackSid() == trackSid) {
            glWidget->setAudioTrackSid(QString());
        }
        break;
    case TrackKind::VIDEO:
        if (glWidget->videoTrackSid() == trackSid) {
            glWidget->setVideoTrackSid(QString());
        }
        break;
    default:
        break;
    }
}

void InMeeting::updateAudioStatusPanel()
{
    if (!meetingSession_) {
        return;
    }

    // Update local participant audio status
    const AudioLevelInfo local_level = meetingSession_->localAudioLevel();
    const bool local_speaking = meetingSession_->isLocalAudioSpeaking();
    const auto localIt = participantWidgets_.find(localParticipantId_);
    if (localIt != participantWidgets_.end() && localIt.value()) {
        localIt.value()->setAudioStatus(local_level.level, local_speaking);
    }

    const QHash<QString, AudioLevelInfo> participantAudioMap = buildRemoteParticipantAudioMap();

    // Update all remote participants' audio status
    for (auto it = participantWidgets_.begin(); it != participantWidgets_.end(); ++it) {
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
            participant->setAudioStatus(audioIt.value().level, audioIt.value().speaking);
        }
    }

    updateMemberAudioBars(local_level, local_speaking, participantAudioMap);
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

    const int contentWidth = qMax(0, width() - leftMargin - rightMargin);
    const int toolbarY = qMax(topMargin, height() - bottomMargin - toolbarHeight);
    const int gridHeight = qMax(0, toolbarY - gap - topMargin);

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
    QVector<ParticipantWidget*> orderedParticipants;
    orderedParticipants.reserve(participantWidgets_.size());

    auto localIt = participantWidgets_.find(localParticipantId_);
    if (localIt != participantWidgets_.end() && localIt.value()) {
        orderedParticipants.push_back(localIt.value());
    }

    QVector<QPair<QString, ParticipantWidget*>> remoteEntries;
    remoteEntries.reserve(participantWidgets_.size());
    for (auto widgetIt = participantWidgets_.cbegin(); widgetIt != participantWidgets_.cend(); ++widgetIt) {
        if (widgetIt.key() != localParticipantId_ && widgetIt.value()) {
            remoteEntries.append(qMakePair(widgetIt.key(), widgetIt.value()));
        }
    }
    std::stable_sort(remoteEntries.begin(), remoteEntries.end(),
                     [](const auto &a, const auto &b) { return a.first < b.first; });
    for (const auto &entry : remoteEntries) {
        orderedParticipants.push_back(entry.second);
    }

    const int n = static_cast<int>(orderedParticipants.size());
    if (n <= 0) return;

    // 列数基于实际有效 widget 数量
    const int cols = qMax(1, qCeil(qSqrt(static_cast<double>(n))));
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
    QVector<GLWidget*> orderedWidgets;
    orderedWidgets.reserve(static_cast<qsizetype>(orderedParticipants.size()));
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