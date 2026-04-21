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
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QLabel>
#include <QTimer>
#include <QSet>

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

    audioStatusPanel_ = new QWidget(this);
    audioStatusPanel_->setObjectName("audioStatusPanel");
    audioStatusPanel_->setStyleSheet(
        "QWidget#audioStatusPanel {"
        " background-color: rgba(20, 24, 30, 170);"
        " border: 1px solid rgba(120, 134, 156, 120);"
        " border-radius: 8px;"
        "}"
        "QLabel { color: #E8EDF5; }"
        "QProgressBar {"
        " border: 1px solid rgba(80, 90, 110, 180);"
        " border-radius: 4px;"
        " background: rgba(8, 10, 14, 150);"
        " text-align: center;"
        " color: #E8EDF5;"
        "}"
        "QProgressBar::chunk {"
        " border-radius: 3px;"
        " background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #27C93F, stop:1 #0B8A2C);"
        "}");

    auto *statusLayout = new QVBoxLayout(audioStatusPanel_);
    statusLayout->setContentsMargins(10, 8, 10, 8);
    statusLayout->setSpacing(6);

    auto *topRowLayout = new QHBoxLayout();
    topRowLayout->setSpacing(8);
    statusLayout->addLayout(topRowLayout);

    localMicLabel_ = new QLabel("本地麦克风", audioStatusPanel_);
    topRowLayout->addWidget(localMicLabel_);

    localMicBar_ = new QProgressBar(audioStatusPanel_);
    localMicBar_->setRange(0, 100);
    localMicBar_->setValue(0);
    localMicBar_->setTextVisible(false);
    localMicBar_->setFixedWidth(110);
    topRowLayout->addWidget(localMicBar_);

    localMicStateLabel_ = new QLabel("未说话", audioStatusPanel_);
    localMicStateLabel_->setMinimumWidth(52);
    topRowLayout->addWidget(localMicStateLabel_);

    remoteTalkerLabel_ = new QLabel("远端发言: 无", audioStatusPanel_);
    remoteTalkerLabel_->setMinimumWidth(170);
    topRowLayout->addWidget(remoteTalkerLabel_, 1);

    remoteAudioListContainer_ = new QWidget(audioStatusPanel_);
    remoteAudioListLayout_ = new QVBoxLayout(remoteAudioListContainer_);
    remoteAudioListLayout_->setContentsMargins(0, 0, 0, 0);
    remoteAudioListLayout_->setSpacing(4);
    statusLayout->addWidget(remoteAudioListContainer_);

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
        localVideoWidget_ = new VideoGLWidget(this);
        localParticipantId_ = localUser->identity();
        videoWidgets_[localParticipantId_] = localVideoWidget_;
    }

    // default unmuted and video on
    meetingEngine_->startAudio();
    ui->muteBtn->setText("静音");

    if (localVideoWidget_) {
        meetingEngine_->startVideo(localVideoWidget_->trackSid());
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
    if (!localVideoWidget_) {
        return;
    }
    if (button->text() == "开启视频") {
        meetingEngine_->startVideo(localVideoWidget_->trackSid());
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
    const QString &participantId, int trackKind)
{
    qInfo() << __FUNCTION__ 
            << "track subscribed, track_sid=" << trackSid
            << "track_name=" << trackName
            << "participant_id=" << participantId
            << "track_kind=" << trackKindToMediaTypeString(static_cast<TrackKind>(trackKind));
    switch (static_cast<TrackKind>(trackKind)) {
    case TrackKind::AUDIO:
        remoteAudioTrackOwners_[trackSid.toStdString()] = participantId.toStdString();
        break;
    case TrackKind::VIDEO:
    {
        auto participantIdStr = participantId.toStdString();
        auto it = videoWidgets_.find(participantIdStr);
        if (it == videoWidgets_.end()) {
            auto *videoWidget = new VideoGLWidget(this);
            videoWidget->setTrackSid(trackSid.toStdString());
            videoWidget->setLocal(false);
            videoWidgets_[participantIdStr] = videoWidget;
        } else if (it->second) {
            it->second->setTrackSid(trackSid.toStdString());
            it->second->setLocal(false);
        }
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

    if (audioStatusPanel_) {
        const int panelWidth = std::clamp(contentWidth / 2, 300, 620);
        const int rowCount = static_cast<int>(remoteAudioRows_.size());
        const int panelHeight = std::clamp(52 + rowCount * 24, 52, 220);
        audioStatusPanel_->setGeometry(leftMargin + contentWidth - panelWidth, topMargin, panelWidth, panelHeight);
        audioStatusPanel_->raise();
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
    if (localMicBar_) {
        localMicBar_->setValue(static_cast<int>(std::clamp(local_level.level * 100.0f, 0.0f, 100.0f)));
    }
    if (localMicStateLabel_) {
        localMicStateLabel_->setText(local_speaking ? "说话中" : "未说话");
        localMicStateLabel_->setStyleSheet(local_speaking ? "color:#27C93F;" : "color:#C8D1E0;");
    }

    const auto remote_levels = meetingEngine_->remoteAudioLevels();
    std::unordered_map<std::string, bool> remote_speaking_by_participant;
    struct ParticipantAudioSnapshot {
        float level = 0.0f;
        float smoothedDb = -100.0f;
        bool speaking = false;
    };
    std::unordered_map<std::string, ParticipantAudioSnapshot> participant_audio;

    float max_remote_db = -100.0f;
    std::string active_talker;

    for (const auto &entry : remote_levels) {
        const auto owner_it = remoteAudioTrackOwners_.find(entry.first);
        const std::string participant = owner_it != remoteAudioTrackOwners_.end() ? owner_it->second : entry.first;

        auto &snapshot = participant_audio[participant];
        snapshot.level = std::max(snapshot.level, entry.second.level);
        snapshot.smoothedDb = std::max(snapshot.smoothedDb, entry.second.smoothed_db);
        snapshot.speaking = snapshot.speaking || entry.second.speaking;

        const bool speaking = entry.second.speaking;
        remote_speaking_by_participant[participant] = remote_speaking_by_participant[participant] || speaking;

        if (entry.second.smoothed_db > max_remote_db) {
            max_remote_db = entry.second.smoothed_db;
            active_talker = participant;
        }
    }

    // 快速检查：如果远端音量和发言状态都没变，只更新本地、活跃发言人和 speaker 高亮
    bool audioStateChanged = false;
    if (lastRemoteAudioState_.size() != participant_audio.size()) {
        audioStateChanged = true;
    } else {
        for (const auto &entry : participant_audio) {
            const auto it = lastRemoteAudioState_.find(entry.first);
            if (it == lastRemoteAudioState_.end() ||
                std::abs(it->second.first - entry.second.level) > 0.01f ||
                it->second.second != entry.second.speaking) {
                audioStateChanged = true;
                break;
            }
        }
    }

    if (audioStateChanged) {
        // 更新缓存状态
        lastRemoteAudioState_.clear();
        for (const auto &entry : participant_audio) {
            lastRemoteAudioState_[entry.first] = {entry.second.level, entry.second.speaking};
        }
    }

    if (remoteAudioListLayout_) {
        QSet<QString> participantsInUse;
        for (const auto &entry : participant_audio) {
            participantsInUse.insert(QString::fromStdString(entry.first));

            auto &row = remoteAudioRows_[entry.first];
            if (!row.row) {
                row.row = new QWidget(remoteAudioListContainer_);
                auto *rowLayout = new QHBoxLayout(row.row);
                rowLayout->setContentsMargins(0, 0, 0, 0);
                rowLayout->setSpacing(6);

                row.nameLabel = new QLabel(QString::fromStdString(entry.first), row.row);
                row.nameLabel->setMinimumWidth(80);
                rowLayout->addWidget(row.nameLabel);

                row.levelBar = new QProgressBar(row.row);
                row.levelBar->setRange(0, 100);
                row.levelBar->setTextVisible(false);
                row.levelBar->setFixedWidth(110);
                rowLayout->addWidget(row.levelBar);

                row.stateLabel = new QLabel("未说话", row.row);
                row.stateLabel->setMinimumWidth(52);
                rowLayout->addWidget(row.stateLabel);

                remoteAudioListLayout_->addWidget(row.row);
            }

            // 仅当状态变化时才更新 UI，避免频繁的字符串转换和样式更新
            if (!audioStateChanged) {
                continue;
            }

            if (row.nameLabel) {
                row.nameLabel->setText(QString::fromStdString(entry.first));
            }
            if (row.levelBar) {
                row.levelBar->setValue(static_cast<int>(std::clamp(entry.second.level * 100.0f, 0.0f, 100.0f)));
            }
            if (row.stateLabel) {
                row.stateLabel->setText(entry.second.speaking ? "说话中" : "未说话");
                row.stateLabel->setStyleSheet(entry.second.speaking ? "color:#27C93F;" : "color:#C8D1E0;");
            }
        }

        std::vector<std::string> staleParticipants;
        staleParticipants.reserve(remoteAudioRows_.size());
        for (const auto &entry : remoteAudioRows_) {
            if (!participantsInUse.contains(QString::fromStdString(entry.first))) {
                staleParticipants.push_back(entry.first);
            }
        }

        for (const auto &participant : staleParticipants) {
            auto it = remoteAudioRows_.find(participant);
            if (it == remoteAudioRows_.end()) {
                continue;
            }
            if (it->second.row) {
                remoteAudioListLayout_->removeWidget(it->second.row);
                delete it->second.row;
            }
            remoteAudioRows_.erase(it);
        }
    }

    if (remoteTalkerLabel_ && audioStateChanged) {
        if (active_talker.empty()) {
            remoteTalkerLabel_->setText("远端发言: 无");
        } else {
            remoteTalkerLabel_->setText(QString("远端发言: %1").arg(QString::fromStdString(active_talker)));
        }
    }

    if (audioStatusPanel_) {
        const int leftMargin = 20;
        const int topMargin = 20;
        const int rightMargin = 20;
        const int contentWidth = std::max(0, width() - leftMargin - rightMargin);
        const int panelWidth = std::clamp(contentWidth / 2, 300, 620);
        const int rowCount = static_cast<int>(remoteAudioRows_.size());
        const int panelHeight = std::clamp(52 + rowCount * 24, 52, 220);
        audioStatusPanel_->setGeometry(leftMargin + contentWidth - panelWidth, topMargin, panelWidth, panelHeight);
    }

    updateSpeakerHighlight(local_speaking, remote_speaking_by_participant);
}

void InMeeting::updateSpeakerHighlight(
    bool localSpeaking,
    const std::unordered_map<std::string, bool> &remoteSpeakingByParticipant)
{
    for (const auto &entry : videoWidgets_) {
        auto *videoWidget = entry.second;
        if (!videoWidget) {
            continue;
        }

        bool speaking = false;
        if (videoWidget->isLocal()) {
            speaking = localSpeaking;
        } else {
            const auto it = remoteSpeakingByParticipant.find(entry.first);
            speaking = it != remoteSpeakingByParticipant.end() && it->second;
        }

        const auto stateIt = lastSpeakingStateByWidget_.find(videoWidget);
        if (stateIt != lastSpeakingStateByWidget_.end() && stateIt->second == speaking) {
            continue;
        }
        lastSpeakingStateByWidget_[videoWidget] = speaking;

        if (speaking) {
            videoWidget->setStyleSheet(
                "border: 2px solid #27C93F;"
                "border-radius: 6px;"
                "background-color: #0D1218;");
        } else {
            videoWidget->setStyleSheet(
                "border: 1px solid #2A3442;"
                "border-radius: 6px;"
                "background-color: #0D1218;");
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
    std::vector<VideoGLWidget*> orderedWidgets;
    orderedWidgets.reserve(videoWidgets_.size());

    auto localIt = videoWidgets_.find(localParticipantId_);
    if (localIt != videoWidgets_.end() && localIt->second) {
        orderedWidgets.push_back(localIt->second);
    }

    std::vector<std::pair<std::string, VideoGLWidget*>> remoteEntries;
    remoteEntries.reserve(videoWidgets_.size());
    for (const auto &entry : videoWidgets_) {
        if (entry.first != localParticipantId_ && entry.second) {
            remoteEntries.emplace_back(entry.first, entry.second);
        }
    }
    std::sort(remoteEntries.begin(), remoteEntries.end(),
              [](const auto &a, const auto &b) { return a.first < b.first; });
    for (const auto &entry : remoteEntries) {
        orderedWidgets.push_back(entry.second);
    }

    const int n = static_cast<int>(orderedWidgets.size());
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
        ui->gridLayout->addWidget(orderedWidgets[i], i / cols, i % cols);
    }

    for (int c = 0; c < cols; ++c) {
        ui->gridLayout->setColumnStretch(c, 1);
    }
    for (int r = 0; r < rows; ++r) {
        ui->gridLayout->setRowStretch(r, 1);
    }

    // 缓存有序 widget 指针供 onTimer 使用
    cachedOrderedWidgets_.swap(orderedWidgets);
}