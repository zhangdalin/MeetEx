#include "inmeeting.h"
#include "ui_inmeeting.h"
#include "meeting_engine.h"
#include "meeting_room.h"
#include "meeting_def.h"
#include "local_user.h"
#include "participantwidget.h"

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
        localVideoWidget_ = new ParticipantWidget(this);
        localParticipantId_ = QString::fromStdString(localUser->identity());
        localVideoWidget_->setParticipantName("我");
        participantWidgets_[localParticipantId_] = localVideoWidget_;
    }

    // default unmuted and video on
    meetingEngine_->startAudio();
    ui->muteBtn->setText("静音");

    if (localVideoWidget_) {
        std::string localVideoSid;
        meetingEngine_->startVideo(localVideoSid);
        localVideoWidget_->setVideoTrackSid(QString::fromStdString(localVideoSid));
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
        std::string localVideoSid;
        meetingEngine_->startVideo(localVideoSid);
        localVideoWidget_->setVideoTrackSid(QString::fromStdString(localVideoSid));
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
        auto *participantWidget = new ParticipantWidget(this);
        participantWidget->setParticipantName(participantName);
        participantWidgets_[participantId] = participantWidget;
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
    ParticipantWidget *participantWidget = nullptr;
    if (it == participantWidgets_.end()) {
        participantWidget = new ParticipantWidget(this);
        participantWidget->setParticipantName(participantId);
        participantWidgets_[participantId] = participantWidget;
    } else {
        participantWidget = it.value();
    }

    if (!participantWidget) {
        return;
    }

    switch (static_cast<TrackKind>(trackKind))
    {
    case TrackKind::AUDIO:
        remoteAudioTrackOwners_[trackSid] = participantId;
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
    if (localVideoWidget_) {
        localVideoWidget_->setAudioStatus(local_level.level, local_speaking);
    }

    const auto remote_levels = meetingEngine_->remoteAudioLevels();
    QHash<QString, bool> remote_speaking_by_participant;
    struct ParticipantAudioSnapshot { float level = 0.0f; bool speaking = false; };
    QHash<QString, ParticipantAudioSnapshot> participant_audio;

    for (const auto &entry : remote_levels) {
        const QString trackSid = QString::fromStdString(entry.first);
        const auto owner_it = remoteAudioTrackOwners_.find(trackSid);
        const QString participant = owner_it != remoteAudioTrackOwners_.end()
            ? owner_it.value()
            : trackSid;

        auto &snapshot = participant_audio[participant];
        snapshot.level = std::max(snapshot.level, entry.second.level);
        snapshot.speaking = snapshot.speaking || entry.second.speaking;

        const bool speaking = entry.second.speaking;
        remote_speaking_by_participant[participant] = remote_speaking_by_participant.value(participant, false) || speaking;
    }

    for (auto widgetIt = participantWidgets_.cbegin(); widgetIt != participantWidgets_.cend(); ++widgetIt) {
        const QString &participantId = widgetIt.key();
        auto *participantWidget = widgetIt.value();
        if (!participantWidget || participantId == localParticipantId_) {
            continue;
        }

        const auto it = participant_audio.find(participantId);
        if (it == participant_audio.end()) {
            participantWidget->setAudioStatus(0.0f, false);
        } else {
            participantWidget->setAudioStatus(it.value().level, it.value().speaking);
        }
    }

    updateSpeakerHighlight(local_speaking, remote_speaking_by_participant);
}

void InMeeting::updateSpeakerHighlight(
    bool localSpeaking,
    const QHash<QString, bool> &remoteSpeakingByParticipant)
{
    for (auto widgetIt = participantWidgets_.cbegin(); widgetIt != participantWidgets_.cend(); ++widgetIt) {
        auto *videoWidget = widgetIt.value();
        const QString participantId = widgetIt.key();
        if (!videoWidget) {
            continue;
        }

        bool speaking = false;
        if (participantId == localParticipantId_) {
            speaking = localSpeaking;
        } else {
            const auto it = remoteSpeakingByParticipant.find(participantId);
            speaking = it != remoteSpeakingByParticipant.end() && it.value();
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
    std::vector<ParticipantWidget*> orderedWidgets;
    orderedWidgets.reserve(participantWidgets_.size());

    auto localIt = participantWidgets_.find(localParticipantId_);
    if (localIt != participantWidgets_.end() && localIt.value()) {
        orderedWidgets.push_back(localIt.value());
    }

    std::vector<std::pair<QString, ParticipantWidget*>> remoteEntries;
    remoteEntries.reserve(participantWidgets_.size());
    for (auto widgetIt = participantWidgets_.cbegin(); widgetIt != participantWidgets_.cend(); ++widgetIt) {
        if (widgetIt.key() != localParticipantId_ && widgetIt.value()) {
            remoteEntries.emplace_back(widgetIt.key(), widgetIt.value());
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