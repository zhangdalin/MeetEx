#include "meeting_session.h"

#include "meeting_engine.h"
#include "meeting_room.h"
#include "remote_user.h"

#include <QDebug>

bool MeetingSessionCtx::isValid() const {
    return !livekitUrl.trimmed().isEmpty() && !livekitToken.trimmed().isEmpty();
}

MeetingSessionCtx MeetingSessionCtx::developmentDefaults() {
    MeetingSessionCtx context;
    context.livekitUrl = QStringLiteral(LIVEKIT_URL);
    context.livekitToken = QStringLiteral(LIVEKIT_TOKEN);
    context.meetingNumber = QStringLiteral("meetex");
    context.displayName = QStringLiteral("Me");
    context.joinOptions.autoConnectAudio = true;
    context.joinOptions.startCamera = true;
    context.joinOptions.startMicrophone = true;
    return context;
}

MeetingSession::MeetingSession(const MeetingSessionCtx &context, QObject *parent)
    : QObject(parent)
    , context_(context)
    , engine_(std::make_unique<MeetingEngine>()) {
    bindEngineSignals();
}

MeetingSession::~MeetingSession() {
    shutdown();
}

bool MeetingSession::start() {
    if (started_) {
        return true;
    }

    if (!context_.isValid()) {
        emit sigSessionError(QStringLiteral("MeetingSessionCtx is invalid."));
        return false;
    }

    setRoomState(MeetingSessionRoomState::Connecting);
    engine_->configureConnection(context_.livekitUrl.toStdString(),
        context_.livekitToken.toStdString());

    if (!engine_->joinMeeting()) {
        setRoomState(MeetingSessionRoomState::Disconnected);
        emit sigSessionError(QStringLiteral("Failed to join meeting room."));
        return false;
    }

    started_ = true;
    setRoomState(MeetingSessionRoomState::Connected);

    if (context_.joinOptions.autoConnectAudio && context_.joinOptions.startMicrophone) {
        startAudio();
    }

    if (context_.joinOptions.startCamera) {
        startVideo();
    }

    return true;
}

void MeetingSession::shutdown() {
    if (!started_) {
        return;
    }

    setRoomState(MeetingSessionRoomState::Disconnecting);
    if (cameraState_ == MeetingSessionMediaState::On) {
        stopVideo();
    }
    if (microphoneState_ == MeetingSessionMediaState::On) {
        stopAudio();
    }
    engine_->endMeeting();
    started_ = false;
    localVideoTrackSid_.clear();
    setRoomState(MeetingSessionRoomState::Disconnected);
}

bool MeetingSession::startAudio() {
    setMicrophoneState(MeetingSessionMediaState::Starting);
    if (!engine_->startAudio()) {
        setMicrophoneState(MeetingSessionMediaState::Failed);
        emit sigSessionError(QStringLiteral("Failed to start microphone."));
        return false;
    }

    setMicrophoneState(MeetingSessionMediaState::On);
    return true;
}

void MeetingSession::stopAudio() {
    setMicrophoneState(MeetingSessionMediaState::Stopping);
    engine_->stopAudio();
    setMicrophoneState(MeetingSessionMediaState::Off);
}

bool MeetingSession::startVideo() {
    std::string localVideoSid;
    setCameraState(MeetingSessionMediaState::Starting);
    if (!engine_->startVideo(localVideoSid)) {
        setCameraState(MeetingSessionMediaState::Failed);
        emit sigSessionError(QStringLiteral("Failed to start camera."));
        return false;
    }

    localVideoTrackSid_ = localVideoSid;
    setCameraState(MeetingSessionMediaState::On);
    return true;
}

void MeetingSession::stopVideo() {
    setCameraState(MeetingSessionMediaState::Stopping);
    engine_->stopVideo();
    localVideoTrackSid_.clear();
    setCameraState(MeetingSessionMediaState::Off);
}

QString MeetingSession::localParticipantId() const {
    return QString::fromStdString(engine_->localUserIdentity());
}

QString MeetingSession::localVideoTrackSid() const {
    return QString::fromStdString(localVideoTrackSid_);
}

AudioLevelInfo MeetingSession::localAudioLevel() const {
    return engine_->localAudioLevel();
}

bool MeetingSession::isLocalAudioSpeaking() const {
    return engine_->isLocalAudioSpeaking();
}

std::unordered_map<std::string, AudioLevelInfo> MeetingSession::remoteAudioLevels() const {
    return engine_->remoteAudioLevels();
}

std::vector<std::shared_ptr<RemoteUser>> MeetingSession::remoteUsers() const {
    return engine_->remoteUsers();
}

void MeetingSession::bindEngineSignals() {
    auto *room = engine_->room();
    QObject::connect(room, &MeetingRoom::sigParticipantJoined,
        this, &MeetingSession::sigParticipantJoined);
    QObject::connect(room, &MeetingRoom::sigParticipantLeft,
        this, &MeetingSession::sigParticipantLeft);
    QObject::connect(room, &MeetingRoom::sigTrackSubscribed,
        this, &MeetingSession::sigTrackSubscribed);
    QObject::connect(room, &MeetingRoom::sigTrackUnsubscribed,
        this, &MeetingSession::sigTrackUnsubscribed);
    QObject::connect(room, &MeetingRoom::sigReconnecting, this, [this]() {
        setRoomState(MeetingSessionRoomState::Reconnecting);
    });
    QObject::connect(room, &MeetingRoom::sigReconnected, this, [this]() {
        setRoomState(MeetingSessionRoomState::Connected);
    });
    QObject::connect(room, &MeetingRoom::sigDisconnected, this,
        [this](livekit::DisconnectReason) {
            started_ = false;
            localVideoTrackSid_.clear();
            setMicrophoneState(MeetingSessionMediaState::Off);
            setCameraState(MeetingSessionMediaState::Off);
            setRoomState(MeetingSessionRoomState::Disconnected);
        });
}

void MeetingSession::setRoomState(MeetingSessionRoomState state) {
    if (roomState_ == state) {
        return;
    }

    roomState_ = state;
    emit sigRoomStateChanged(roomState_);
}

void MeetingSession::setMicrophoneState(MeetingSessionMediaState state) {
    if (microphoneState_ == state) {
        return;
    }

    microphoneState_ = state;
    emit sigMicrophoneStateChanged(microphoneState_);
}

void MeetingSession::setCameraState(MeetingSessionMediaState state) {
    if (cameraState_ == state) {
        return;
    }

    cameraState_ = state;
    emit sigCameraStateChanged(cameraState_);
}