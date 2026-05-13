#include "inmeeting.h"
#include "ui_inmeeting.h"
#include "meeting_session.h"
#include "meeting_def.h"
#include "meeting_participant.h"
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
#include <QThread>

extern std::unique_ptr<QWidget> home;
extern std::unique_ptr<QWidget> myprofile;
extern std::unique_ptr<QWidget> joinmeeting;
extern std::unique_ptr<QWidget> inmeeting;
extern std::unique_ptr<QWidget> bookmeeting;
extern std::unique_ptr<QWidget> sharescreen;
extern std::unique_ptr<QWidget> settings;

static const char* trackKindToString(TrackKind track_kind) {
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
    , localParticipantWidget_(nullptr)
    , meetingSession_(new MeetingSession(MeetingSessionCtx::defaults(), this))
{
    ui->setupUi(this);

    // Connect participant signals
    connect(meetingSession_, &MeetingSession::sigParticipantJoined,
            this, &InMeeting::onParticipantJoined);
    connect(meetingSession_, &MeetingSession::sigParticipantLeft,
            this, &InMeeting::onParticipantLeft);
    connect(meetingSession_, &MeetingSession::sigTrackSubscribed,
            this, &InMeeting::onTrackSubscribed);
    connect(meetingSession_, &MeetingSession::sigTrackUnsubscribed,
            this, &InMeeting::onTrackUnsubscribed);

    // Connect state change signals for UI updates
    connect(meetingSession_, &MeetingSession::sigMicrophoneStateChanged,
            this, &InMeeting::updateButtonStates);
    connect(meetingSession_, &MeetingSession::sigCameraStateChanged,
            this, &InMeeting::updateButtonStates);

    // Start the meeting session
    if (meetingSession_->start()) {
        auto localParticipant = meetingSession_->localParticipant();
        localParticipantWidget_ = new ParticipantWidget(localParticipant, this);
    }

    ui->tabWidget->setVisible(false);
    updateButtonStates();

    // UI 首帧后同步侧栏内容尺寸，避免首次展开 memberlist 时几何仍是 .ui 初始值
    QTimer::singleShot(0, this, [this]() {
        updateSidePanelGeometry();
    });

    // Timer for video rendering and periodic updates
    auto *timer = new QTimer(this);
    timer->setInterval(16);
    connect(timer, &QTimer::timeout, this, &InMeeting::onTimer);
    timer->start();
}

InMeeting::~InMeeting()
{
    delete ui;
}

void InMeeting::updateButtonStates()
{
    const auto micState = meetingSession_->microphoneState();
    const auto camState = meetingSession_->cameraState();
    const auto screenShareState = meetingSession_->screenShareState();
    const auto recordingState = meetingSession_->recordingState();

    ui->micBtn->setText(micState == MeetingSessionMediaState::On ? "关闭麦克风" : "开启麦克风");
    ui->camBtn->setText(camState == MeetingSessionMediaState::On ? "关闭摄像头" : "开启摄像头");
    ui->shareBtn->setText(screenShareState == MeetingSessionMediaState::On ? "停止共享" : "开始共享");
    ui->recordBtn->setText(recordingState == MeetingSessionMediaState::On ? "停止录制" : "开始录制");
}

void InMeeting::toggleAudio()
{
    qInfo() << QThread::currentThread() << __FUNCTION__;
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (meetingSession_->microphoneState() == MeetingSessionMediaState::On) {
        meetingSession_->stopAudio();
    } else {
        meetingSession_->startAudio();
    }

    button->setText(meetingSession_->microphoneState() == MeetingSessionMediaState::On ? "关闭麦克风" : "开启麦克风");

}

void InMeeting::toggleVideo()
{
    qInfo() << QThread::currentThread() << __FUNCTION__;
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (meetingSession_->cameraState() != MeetingSessionMediaState::On) {
        meetingSession_->startVideo();
    } else {
        meetingSession_->stopVideo();
    }

    button->setText(meetingSession_->cameraState() == MeetingSessionMediaState::On ? "关闭摄像头" : "开启摄像头");

}

void InMeeting::toggleRecord()
{
    qInfo() << QThread::currentThread() << __FUNCTION__;
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (meetingSession_->recordingState() != MeetingSessionMediaState::On) {
        meetingSession_->startRecording();
    } else {
        meetingSession_->stopRecording();
    }

    button->setText(meetingSession_->recordingState() == MeetingSessionMediaState::On ? "停止录制" : "开始录制");
}

void InMeeting::toggleShare()
{
    qInfo() << QThread::currentThread() << __FUNCTION__;
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (meetingSession_->screenShareState() != MeetingSessionMediaState::On) {
        meetingSession_->startShare();
    } else {
        meetingSession_->stopShare();
    }

    button->setText(meetingSession_->screenShareState() == MeetingSessionMediaState::On ? "停止共享" : "开始共享");
}

void InMeeting::sendMsg()
{
    qInfo() << QThread::currentThread() << __FUNCTION__;
}

void InMeeting::toggleMember()
{
    qInfo() << QThread::currentThread() << __FUNCTION__;
    const bool visible = !ui->tabWidget->isVisible() || ui->tabWidget->currentIndex() != 0;
    if (visible) {
        ui->tabWidget->setCurrentIndex(0);
        ui->tabWidget->setVisible(true);
        updateSidePanelGeometry();
        return;
    }

    ui->tabWidget->setVisible(false);
}

void InMeeting::inviteUser()
{
    qInfo() << QThread::currentThread() << __FUNCTION__;
}

void InMeeting::toggleChat()
{
    qInfo() << QThread::currentThread() << __FUNCTION__;
    const bool visible = ui->tabWidget->isVisible() || ui->tabWidget->currentIndex() != 1;
     if (visible) {
        ui->tabWidget->setCurrentIndex(1);
        ui->tabWidget->setVisible(true);
        updateSidePanelGeometry();
        return;
    }

    ui->tabWidget->setVisible(false);
}

void InMeeting::openApps()
{
    qInfo() << QThread::currentThread() << __FUNCTION__;
}

void InMeeting::endMeeting()
{
    qInfo() << QThread::currentThread() << __FUNCTION__;
    close();
}

void InMeeting::onParticipantJoined(const QString &participantId)
{
    qInfo() << QThread::currentThread() << __FUNCTION__
            << "new participant joined, id=" << participantId;
    if (participantId.isEmpty()) {
        qWarning() << QThread::currentThread() << __FUNCTION__ << "participantId is empty, ignoring";
        return;
    }

    const auto &remoteParticipants = meetingSession_->remoteParticipants();
    if (!participantWidgets_.contains(participantId)) {
        participantWidgets_.emplace(participantId, new ParticipantWidget(remoteParticipants[participantId], this));
    }
    else {
        qWarning() << QThread::currentThread() << __FUNCTION__ << "participant widget with id" << participantId << "already exists, ignoring";
    }

    updateParticipantWidgets();
}

void InMeeting::onParticipantLeft(const QString &participantId)
{
    qInfo() << QThread::currentThread() << __FUNCTION__
            << "participant left, id=" << participantId;
    if (participantId.isEmpty()) {
        qWarning() << QThread::currentThread() << __FUNCTION__ << "participantId is empty, ignoring";
        return;
    }

    const auto &remoteParticipants = meetingSession_->remoteParticipants();
    if (!participantWidgets_.contains(participantId)) {
        qWarning() << QThread::currentThread() << __FUNCTION__ << "participant widget with id" << participantId << "not found, ignoring";
    }
    else {
        participantWidgets_[participantId]->deleteLater();
        participantWidgets_.remove(participantId);
    }

    updateParticipantWidgets();
}

void InMeeting::onTrackSubscribed(const QString &participantId, int trackKind)
{
    qInfo() << QThread::currentThread() << __FUNCTION__
            << "track subscribed, participant_id=" << participantId
            << "track_kind=" << trackKindToString(static_cast<TrackKind>(trackKind));
    if (participantId.isEmpty()) {
        qWarning() << QThread::currentThread() << __FUNCTION__ << "participantId is empty, ignoring";
        return;
    }

    const auto &remoteParticipants = meetingSession_->remoteParticipants();
    if (!participantWidgets_.contains(participantId)) {
        participantWidgets_.emplace(participantId, new ParticipantWidget(remoteParticipants[participantId], this));
    }
    const TrackKind kind = static_cast<TrackKind>(trackKind);
    if (kind == TrackKind::AUDIO) {
        participantWidgets_[participantId]->setAudioTrackSid(remoteParticipants[participantId].audioTrackSid());
    } else if (kind == TrackKind::VIDEO) {
        participantWidgets_[participantId]->setVideoTrackSid(remoteParticipants[participantId].videoTrackSid());
    }

    updateParticipantWidgets();
}

void InMeeting::onTrackUnsubscribed(const QString &participantId, int trackKind)
{
    qInfo() << QThread::currentThread() << __FUNCTION__
            << "track unsubscribed, participant_id=" << participantId
            << "track_kind=" << trackKindToString(static_cast<TrackKind>(trackKind));
    if (participantId.isEmpty()) {
        qWarning() << QThread::currentThread() << __FUNCTION__ << "participantId is empty, ignoring";
        return;
    }

    const auto &remoteParticipants = meetingSession_->remoteParticipants();
    const TrackKind kind = static_cast<TrackKind>(trackKind);
    if (kind == TrackKind::AUDIO) {
        participantWidgets_[participantId]->setAudioTrackSid(QString());
        participantWidgets_[participantId]->setAudioStatus(0.0f, false);
    } else if (kind == TrackKind::VIDEO) {
        participantWidgets_[participantId]->setVideoTrackSid(QString());
    }

    updateParticipantWidgets();
}

void InMeeting::updateParticipantWidgets()
{
    qInfo() << QThread::currentThread() << __FUNCTION__;

    // 更新成员列表
    ui->memberListWidget->clear();

    // 先从 layout 移除（不 delete 控件，控件仍由父对象管理）
    while (QLayoutItem *item = ui->userGridLayout->takeAt(0)) {
        delete item;
    }

    // 本地优先，远端按 id 排序——单次遍历直接收集指针，避免二次 find()
    QVector<ParticipantWidget*> showParticipants;
    showParticipants.reserve(participantWidgets_.size() + 1);
    showParticipants.append(localParticipantWidget_);
    showParticipants.append(participantWidgets_.values().toVector());

    // 缓存成员小组件列表，供 updateAudioStatusPanel 高效查询
    QVector<MemberWidget*> memberWidgets;
    memberWidgets.reserve(showParticipants.size());

    for (auto *participant : showParticipants) {
        if (participant) {
            const bool isLocalUser = (participant == localParticipantWidget_);
            MemberWidget* memberWidget = new MemberWidget(participant->id(), participant->name(), isLocalUser, ui->memberListWidget);
            auto *item = new QListWidgetItem(ui->memberListWidget);
            item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
            item->setSizeHint(memberWidget->sizeHint());
            ui->memberListWidget->addItem(item);
            ui->memberListWidget->setItemWidget(item, memberWidget);
            memberWidgets.push_back(memberWidget);
        }
    }
    cachedMemberWidgets_.swap(memberWidgets);
    
    const int n = static_cast<int>(showParticipants.size());

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
        ui->userGridLayout->addWidget(showParticipants[i], i / cols, i % cols);
    }

    for (int c = 0; c < cols; ++c) {
        ui->userGridLayout->setColumnStretch(c, 1);
    }
    for (int r = 0; r < rows; ++r) {
        ui->userGridLayout->setRowStretch(r, 1);
    }

    // 缓存有序 GLWidget 指针供 onTimer 使用
    QVector<GLWidget*> glWidgets;
    glWidgets.reserve(static_cast<qsizetype>(showParticipants.size()));
    for (auto *participant : showParticipants) {
        if (participant) {
            GLWidget *glWidget = participant->getGLWidget();
            if (glWidget) {
                glWidgets.push_back(glWidget);
            }
        }
    }
    cachedOrderedWidgets_.swap(glWidgets);
}


void InMeeting::updateAudioStatusPanel()
{
    qInfo() << QThread::currentThread() << __FUNCTION__;

    if (!meetingSession_) {
        return;
    }

    // 用单个哈希表合并音频状态，减少查询
    struct AudioStatus {
        float level = 0.0f;
        bool speaking = false;
    };
    QHash<QString, AudioStatus> audioStatusByParticipant;

    // update local audio status
    const auto localAudio = meetingSession_->localAudioLevel();
    if (localParticipantWidget_) {
        const float level = localAudio.speaking ? localAudio.level : 0.0f;
        localParticipantWidget_->setAudioStatus(level, localAudio.speaking);
        audioStatusByParticipant.insert(localParticipantWidget_->id(), 
                                        {level, localAudio.speaking});
    }

    // update remote audio status
    auto remoteLevels = meetingSession_->remoteAudioLevels();
    for (auto it = remoteLevels.begin(); it != remoteLevels.end(); ++it) {
        auto participant = meetingSession_->findParticipantByTrackSid(QString::fromStdString(it->first), 
            static_cast<int>(livekit::TrackKind::KIND_AUDIO));
        if (!participant) {
            continue;
        }

        const QString participantId = participant->id();
        const float level = it->second.speaking ? it->second.level : 0.0f;
        audioStatusByParticipant.insert(participantId, {level, it->second.speaking});

        if (participantWidgets_.contains(participantId)) {
            participantWidgets_[participantId]->setAudioStatus(level, it->second.speaking);
        }
    }

    // update member list audio status using cached widget pointers (O(n) instead of O(n²))
    for (auto *memberWidget : cachedMemberWidgets_) {
        if (!memberWidget) {
            continue;
        }
        const QString memberId = memberWidget->memberId();
        const auto status = audioStatusByParticipant.value(memberId, {0.0f, false});
        memberWidget->setAudioStatus(status.level, status.speaking);
    }
}

void InMeeting::closeEvent(QCloseEvent *event)
{
    if (meetingSession_) {
        meetingSession_->shutdown();
    }
    emit sigClosing();
    QWidget::closeEvent(event);
}

void InMeeting::updateSidePanelGeometry()
{
    if (ui->memberTab && ui->memberListWidget) {
        ui->memberListWidget->setGeometry(ui->memberTab->rect());
    }
    if (ui->chartTab && ui->chatListWidget) {
        ui->chatListWidget->setGeometry(ui->chartTab->rect());
    }
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

    updateSidePanelGeometry();
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
