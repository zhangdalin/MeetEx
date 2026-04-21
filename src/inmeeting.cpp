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
        remoteAudioTrackOwners_[trackSid.toStdString()] = participantIdentity.toStdString();
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

    updateAudioStatusPanel();
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

    if (remoteTalkerLabel_) {
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
    for (auto *videoWidget : videoWidgets_) {
        if (!videoWidget) {
            continue;
        }

        bool speaking = false;
        if (videoWidget->isLocal()) {
            speaking = localSpeaking;
        } else {
            const auto it = remoteSpeakingByParticipant.find(videoWidget->participantIdentity());
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