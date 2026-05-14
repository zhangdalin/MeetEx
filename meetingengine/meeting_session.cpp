#include "meeting_session.h"

#include "media_engine.h"
#include "media_util.h"

#include "livekit/local_participant.h"
#include "livekit/remote_participant.h"

#include <QDebug>
#include <QThread>

QString redactTokenForLog(const std::string &token) {
    if (token.empty()) {
        return QString();
    }

    constexpr int kVisiblePrefix = 6;
    constexpr int kVisibleSuffix = 4;
    const QString value = QString::fromStdString(token);
    if (value.size() <= (kVisiblePrefix + kVisibleSuffix)) {
        return QStringLiteral("<redacted>");
    }

    return value.left(kVisiblePrefix) + QStringLiteral("...") + value.right(kVisibleSuffix);
}

const char* disconnectReasonToString(livekit::DisconnectReason reason) {
    switch (reason) {
        case livekit::DisconnectReason::Unknown: return "Unknown";
        case livekit::DisconnectReason::ClientInitiated: return "ClientInitiated";
        case livekit::DisconnectReason::DuplicateIdentity: return "DuplicateIdentity";
        case livekit::DisconnectReason::ServerShutdown: return "ServerShutdown";
        case livekit::DisconnectReason::ParticipantRemoved: return "ParticipantRemoved";
        case livekit::DisconnectReason::RoomDeleted: return "RoomDeleted";
        case livekit::DisconnectReason::StateMismatch: return "StateMismatch";
        case livekit::DisconnectReason::JoinFailure: return "JoinFailure";
        case livekit::DisconnectReason::Migration: return "Migration";
        case livekit::DisconnectReason::SignalClose: return "SignalClose";
        case livekit::DisconnectReason::RoomClosed: return "RoomClosed";
        case livekit::DisconnectReason::UserUnavailable: return "UserUnavailable";
        case livekit::DisconnectReason::UserRejected: return "UserRejected";
        case livekit::DisconnectReason::SipTrunkFailure: return "SipTrunkFailure";
        case livekit::DisconnectReason::ConnectionTimeout: return "ConnectionTimeout";
        case livekit::DisconnectReason::MediaFailure: return "MediaFailure";
        default: return "Unknown";
    }
}

const char* connectionQualityToString(livekit::ConnectionQuality quality) {
    switch (quality) {
        case livekit::ConnectionQuality::Poor: return "Poor";
        case livekit::ConnectionQuality::Good: return "Good";
        case livekit::ConnectionQuality::Excellent: return "Excellent";
        case livekit::ConnectionQuality::Lost: return "Lost";
        default: return "Unknown";
    }
}

const char* connectionStateToString(livekit::ConnectionState state) {
    switch (state) {
        case livekit::ConnectionState::Disconnected: return "Disconnected";
        case livekit::ConnectionState::Connected: return "Connected";
        case livekit::ConnectionState::Reconnecting: return "Reconnecting";
        default: return "Unknown";
    }
}

bool MeetingSessionCtx::isValid() const {
    return !livekitUrl.trimmed().isEmpty() && !livekitToken.trimmed().isEmpty();
}

MeetingSessionCtx MeetingSessionCtx::defaults() {
    MeetingSessionCtx context;
    context.livekitUrl = QStringLiteral(LIVEKIT_URL);
    context.livekitToken = QStringLiteral(LIVEKIT_TOKEN);
    context.meetingNumber = QStringLiteral("meetex");
    context.displayName = QStringLiteral("我");
    context.joinOptions.startCamera = true;
    context.joinOptions.startMicrophone = true;
    return context;
}

MeetingSession::MeetingSession(const MeetingSessionCtx &context, QObject *parent)
    : QObject(parent)
    , context_(context) {
    MediaEngine::instance().init();
    room_.setDelegate(this);
    localParticipant_.setIsLocal(true);
}

MeetingSession::~MeetingSession() {
    shutdown();
    MediaEngine::instance().fini();
}

bool MeetingSession::start() {
    if (roomState_ != MeetingSessionRoomState::Disconnected) {
        return true;
    }

    if (!context_.isValid()) {
        emit sigSessionError(QStringLiteral("MeetingSessionCtx is invalid."));
        return false;
    }

    setRoomOptions();

    if (!connectRoom()) {
        setRoomState(MeetingSessionRoomState::Disconnected);
        emit sigSessionError(QStringLiteral("Failed to join meeting room."));
        return false;
    }

    setRoomState(MeetingSessionRoomState::Connected);

    // Once connected, sync meeting participants and media state
    syncLocalParticipant();
    syncRemoteParticipants();

    if (shouldStartMicrophoneOnJoin()) {
        startAudio();
    }

    if (shouldStartCameraOnJoin()) {
        startVideo();
    }

    return true;
}

void MeetingSession::shutdown() {
    const bool hasActiveRoom = roomState_ != MeetingSessionRoomState::Disconnected;
    const bool hasLocalMedia = microphoneState_ != MeetingSessionMediaState::Off
        || cameraState_ != MeetingSessionMediaState::Off
        || screenShareState_ != MeetingSessionMediaState::Off
        || !localParticipant_.audioTrackSid().isEmpty()
        || !localParticipant_.videoTrackSid().isEmpty()
        || !screenShareTrackSid_.empty();

    if (!hasActiveRoom && !hasLocalMedia) {
        return;
    }

    if (hasActiveRoom) {
        setRoomState(MeetingSessionRoomState::Disconnecting);
    }

    disconnectRoom();
    stopShare();

    livekit::shutdown();
}

bool MeetingSession::startAudio() {
    auto* localParticipant = room_.localParticipant();
    if (!localParticipant) {
        setMicrophoneState(MeetingSessionMediaState::Failed);
        emit sigSessionError(QStringLiteral("No local user available to start microphone."));
        return false;
    }

    setMicrophoneState(MeetingSessionMediaState::Starting);
    std::string localAudioSidStd;
    if (!MediaEngine::instance().startLocalAudio(localParticipant, localAudioSidStd)) {
        setMicrophoneState(MeetingSessionMediaState::Failed);
        emit sigSessionError(QStringLiteral("Failed to start microphone."));
        localParticipant_.setAudioTrackSid(QString());
        return false;
    }
    localParticipant_.setAudioTrackSid(QString::fromStdString(localAudioSidStd));

    setMicrophoneState(MeetingSessionMediaState::On);
    return true;
}

void MeetingSession::stopAudio() {
    if (microphoneState_ == MeetingSessionMediaState::Off && localParticipant_.audioTrackSid().isEmpty()) {
        return;
    }

    livekit::LocalParticipant *localParticipant = nullptr;
    if (roomState_ == MeetingSessionRoomState::Connected) {
        localParticipant = room_.localParticipant();
    }

    setMicrophoneState(MeetingSessionMediaState::Stopping);
    MediaEngine::instance().stopLocalAudio(localParticipant, localParticipant_.audioTrackSid().toStdString());
    localParticipant_.setAudioTrackSid(QString());
    setMicrophoneState(MeetingSessionMediaState::Off);
}

bool MeetingSession::startVideo() {
    auto *localParticipant = room_.localParticipant();
    if (!localParticipant) {
        setCameraState(MeetingSessionMediaState::Failed);
        emit sigSessionError(QStringLiteral("No local user available to start camera."));
        return false;
    }

    setCameraState(MeetingSessionMediaState::Starting);
    std::string localVideoSidStd;
    if (!MediaEngine::instance().startLocalVideo(localParticipant, localVideoSidStd)) {
        setCameraState(MeetingSessionMediaState::Failed);
        emit sigSessionError(QStringLiteral("Failed to start camera."));
        localParticipant_.setVideoTrackSid(QString());
        return false;
    }
    localParticipant_.setVideoTrackSid(QString::fromStdString(localVideoSidStd));

    setCameraState(MeetingSessionMediaState::On);
    return true;
}

void MeetingSession::stopVideo() {
    if (cameraState_ == MeetingSessionMediaState::Off && localParticipant_.videoTrackSid().isEmpty()) {
        return;
    }

    livekit::LocalParticipant *localParticipant = nullptr;
    if (roomState_ == MeetingSessionRoomState::Connected) {
        localParticipant = room_.localParticipant();
    }

    setCameraState(MeetingSessionMediaState::Stopping);
    MediaEngine::instance().stopLocalVideo(localParticipant, localParticipant_.videoTrackSid().toStdString());
    localParticipant_.setVideoTrackSid(QString());
    setCameraState(MeetingSessionMediaState::Off);
}

bool MeetingSession::startShare() {
    if (roomState_ != MeetingSessionRoomState::Connected) {
        emit sigSessionError(QStringLiteral("Cannot start screen share before room is connected."));
        return false;
    }

    auto *localParticipant = room_.localParticipant();
    if (!localParticipant) {
        setScreenShareState(MeetingSessionMediaState::Failed);
        emit sigSessionError(QStringLiteral("No local user available to start screen share."));
        return false;
    }

    setScreenShareState(MeetingSessionMediaState::Starting);
    std::string localShareSid;
    if (!MediaEngine::instance().startShareLocalScreen(localParticipant, localShareSid)) {
        setScreenShareState(MeetingSessionMediaState::Failed);
        emit sigSessionError(QStringLiteral("Failed to start screen share."));
        screenShareTrackSid_.clear();
        return false;
    }

    screenShareTrackSid_ = localShareSid;
    setScreenShareState(MeetingSessionMediaState::On);
    return true;
}

void MeetingSession::stopShare() {
    if (screenShareState_ == MeetingSessionMediaState::Off && screenShareTrackSid_.empty()) {
        return;
    }

    auto *localParticipant = roomState_ == MeetingSessionRoomState::Connected ? room_.localParticipant() : nullptr;
    MediaEngine::instance().stopShareLocalScreen(localParticipant, screenShareTrackSid_);
    screenShareTrackSid_.clear();
    setScreenShareState(MeetingSessionMediaState::Off);
}

bool MeetingSession::startRecording() {
    setRecordingState(MeetingSessionMediaState::On);
    return true;
}

void MeetingSession::stopRecording() {
    setRecordingState(MeetingSessionMediaState::Off);
}

AudioLevelInfo MeetingSession::localAudioLevel() const {
    return MediaEngine::instance().localAudioLevel();
}

bool MeetingSession::isLocalAudioSpeaking() const {
    return MediaEngine::instance().isLocalAudioSpeaking();
}

std::unordered_map<std::string, AudioLevelInfo> MeetingSession::remoteAudioLevels() const {
    return MediaEngine::instance().remoteAudioLevels();
}

const MeetingParticipant* MeetingSession::findParticipantByTrackSid(const QString &trackSid, int trackKind) const {
    for (const auto &participant : remoteParticipants_) {
        if (static_cast<int>(livekit::TrackKind::KIND_AUDIO) == trackKind &&
            participant.audioTrackSid() == trackSid) {
            return &participant;
        }
        if (static_cast<int>(livekit::TrackKind::KIND_VIDEO) == trackKind &&
            participant.videoTrackSid() == trackSid) {
            return &participant;
        }
    }
    return nullptr;
}

void MeetingSession::setRoomOptions(bool autoSubscribe, bool dynacast, bool e2ee,
    bool singlePeerConnection) {
    roomOptions_.auto_subscribe = autoSubscribe;
    roomOptions_.dynacast = dynacast;
    roomOptions_.single_peer_connection = singlePeerConnection;

    if (e2ee) {
        livekit::E2EEOptions encryption;
        encryption.encryption_type = livekit::EncryptionType::GCM;
        if (!e2eeKey_.isEmpty()) {
            encryption.key_provider_options.shared_key = toBytes(e2eeKey_.toStdString());
        }
        roomOptions_.encryption = encryption;
    }

    qInfo() << QThread::currentThread() << __FUNCTION__ << "set room option with Url:" << context_.livekitUrl << "\n"
        << "Token:" << redactTokenForLog(context_.livekitToken.toStdString()) << "\n"
        << "Auto Subscribe:" << (autoSubscribe ? "enabled" : "disabled") << "\n"
        << "Dynacast:" << (dynacast ? "enabled" : "disabled") << "\n"
        << "Single Peer Connection:" << (singlePeerConnection ? "enabled" : "disabled") << "\n"
        << "E2EE:" << (e2ee ? "enabled" : "disabled");
}

bool MeetingSession::connectRoom() {
    setRoomState(MeetingSessionRoomState::Connecting);
    room_.setDelegate(this);
    const bool res = room_.Connect(context_.livekitUrl.toStdString(),
        context_.livekitToken.toStdString(), roomOptions_);
    qInfo() << QThread::currentThread() << __FUNCTION__ << "Connect result is" << (res ? "successful" : "failed");
    if (!res) {
        setRoomState(MeetingSessionRoomState::Disconnected);
        qCritical() << QThread::currentThread() << __FUNCTION__ << "Failed to connect to room";
        livekit::shutdown();
        return false;
    }

    const auto info = room_.room_info();
    qInfo() << QThread::currentThread() << __FUNCTION__ << "Connected to room:"
        << "SID:" << (info.sid ? QString::fromStdString(*info.sid) : QStringLiteral("(none)"))
        << "Name:" << QString::fromStdString(info.name)
        << "Metadata:" << QString::fromStdString(info.metadata)
        << "Max participants:" << info.max_participants
        << "Num participants:" << info.num_participants
        << "Num publishers:" << info.num_publishers
        << "Active recording:" << (info.active_recording ? "yes" : "no")
        << "Empty timeout (s):" << info.empty_timeout
        << "Departure timeout (s):" << info.departure_timeout
        << "Lossy DC low threshold:" << info.lossy_dc_buffered_amount_low_threshold
        << "Reliable DC low threshold:" << info.reliable_dc_buffered_amount_low_threshold
        << "Creation time (ms):" << info.creation_time;

    return true;
}

void MeetingSession::disconnectRoom() {

    // close local media
    stopAudio();
    stopVideo();
    stopShare();

    MediaEngine::instance().stopAllAudioPlay();
    MediaEngine::instance().stopAllVideoRender();

    if (roomState_ == MeetingSessionRoomState::Disconnected) {
        qWarning() << QThread::currentThread() << __FUNCTION__ << "called but already disconnected";
        return;
    }

    room_.setDelegate(nullptr);
    qInfo() << QThread::currentThread() << __FUNCTION__ << "Disconnected from room";
    setRoomState(MeetingSessionRoomState::Disconnected);
}

void MeetingSession::syncLocalParticipant() {
    if (roomState_ != MeetingSessionRoomState::Connected) {
        qWarning() << QThread::currentThread() << __FUNCTION__ << "called but not connected to room";
        return;
    }
    if (!localParticipant_.isValid()) {
        auto *localParticipant = room_.localParticipant();
        if (!localParticipant) {
            qWarning() << QThread::currentThread() << __FUNCTION__ << "room has no local participant";
            return;
        }
        localParticipant_.syncFromLivekit(localParticipant);
    }
}

void MeetingSession::syncRemoteParticipants() {
    if (roomState_ != MeetingSessionRoomState::Connected) {
        qWarning() << QThread::currentThread() << __FUNCTION__ << "called but not connected to room";
        return;
    }

    QHash<QString, MeetingParticipant> remoteParticipants;
    auto remoteParticipantsVec = room_.remoteParticipants();
    for (auto &participant : remoteParticipantsVec) {
        if (participant) {
            remoteParticipants.insert(QString::fromStdString(participant->sid()),
                MeetingParticipant(participant.get(), false));
        }
    }
    remoteParticipants_.swap(remoteParticipants);
}

bool MeetingSession::shouldStartMicrophoneOnJoin() const {
    return context_.joinOptions.startMicrophone;
}

bool MeetingSession::shouldStartCameraOnJoin() const {
    return context_.joinOptions.startCamera;
}

bool MeetingSession::shouldStartScreenShareOnJoin() const {
    return context_.joinOptions.startScreenShare;
}

bool MeetingSession::shouldStartRecordingOnJoin() const {
    return context_.joinOptions.startRecording;
}

QStringList MeetingSession::buildChangedAttributes(
    const QVector<livekit::AttributeEntry> &attributes) const {
    QStringList changedAttributes;
    changedAttributes.reserve(attributes.size());
    for (const auto &attr : attributes) {
        changedAttributes << QString::fromStdString(attr.key + "=" + attr.value);
    }
    return changedAttributes;
}

void MeetingSession::logRoomSnapshot(const char *eventName, const livekit::RoomInfoData &info) const {
    qDebug() << eventName << "SID="
        << (info.sid ? QString::fromStdString(*info.sid) : QStringLiteral("(none)"))
        << "Name:" << QString::fromStdString(info.name)
        << "Metadata:" << QString::fromStdString(info.metadata)
        << "Max participants:" << info.max_participants
        << "Num participants:" << info.num_participants
        << "Num publishers:" << info.num_publishers
        << "Active recording:" << (info.active_recording ? "yes" : "no")
        << "Empty timeout (s):" << info.empty_timeout
        << "Departure timeout (s):" << info.departure_timeout
        << "Lossy DC low threshold:" << info.lossy_dc_buffered_amount_low_threshold
        << "Reliable DC low threshold:" << info.reliable_dc_buffered_amount_low_threshold
        << "Creation time (ms):" << info.creation_time;
}

void MeetingSession::startRemoteTrackMedia(const std::shared_ptr<livekit::Track> &track) const {
    if (!track) {
        return;
    }

    if (track->kind() == livekit::TrackKind::KIND_VIDEO) {
        livekit::VideoStream::Options opts;
        opts.format = livekit::VideoBufferType::RGBA;
        auto videoStream = livekit::VideoStream::fromTrack(track, opts);
        if (!videoStream) {
            qCritical() << QThread::currentThread() << __FUNCTION__ << "Failed to create VideoStream for track" << QString::fromStdString(track->sid());
            return;
        }
        if (!MediaEngine::instance().startVideoRender(videoStream, track->sid())) {
            qCritical() << QThread::currentThread() << __FUNCTION__ << "video render failed for track" << QString::fromStdString(track->sid());
        }
        return;
    }

    if (track->kind() == livekit::TrackKind::KIND_AUDIO) {
        livekit::AudioStream::Options opts;
        auto audioStream = livekit::AudioStream::fromTrack(track, opts);
        if (!audioStream) {
            qCritical() << QThread::currentThread() << __FUNCTION__ << "Failed to create AudioStream for track" << QString::fromStdString(track->sid());
            return;
        }
        if (!MediaEngine::instance().startAudioPlay(audioStream, track->sid())) {
            qCritical() << QThread::currentThread() << __FUNCTION__ << "audio play failed for track" << QString::fromStdString(track->sid());
        }
    }
}

void MeetingSession::stopRemoteTrackMedia(const std::shared_ptr<livekit::Track> &track) const {
    if (!track) {
        return;
    }

    if (track->kind() == livekit::TrackKind::KIND_VIDEO) {
        MediaEngine::instance().stopVideoRender(track->sid());
        return;
    }

    if (track->kind() == livekit::TrackKind::KIND_AUDIO) {
        MediaEngine::instance().stopAudioPlay(track->sid());
    }
}

void MeetingSession::onParticipantConnected(livekit::Room &room, const livekit::ParticipantConnectedEvent &ev) {
    Q_UNUSED(room);
    if (!ev.participant) {
        qWarning() << QThread::currentThread() << __FUNCTION__ << "participant connected event without participant";
        return;
    }

    const QString participantId = ev.participant ? QString::fromStdString(ev.participant->identity()) : QString();
    const QString name = ev.participant ? QString::fromStdString(ev.participant->name()) : QString();
    qDebug() << QThread::currentThread() << __FUNCTION__ << "participant connected: id=" << participantId
        << "name=" << name;
    
    if (participantId.isEmpty()) {
        qWarning() << QThread::currentThread() << __FUNCTION__ << "participant connected event with empty participant id";
        return;
    }
    
    if (remoteParticipants_.contains(participantId)) {
        // need to update existing participant in case of reconnection with same identity
        qWarning() << QThread::currentThread() << __FUNCTION__ << "participant with id" << participantId << "already exists, update remote participants";
        remoteParticipants_[participantId].syncFromLivekit(ev.participant);
    } else {
        // new remote participant, add to remoteParticipants
        remoteParticipants_.emplace(participantId, MeetingParticipant(ev.participant, false));
    }

    emit sigParticipantJoined(participantId);
}

void MeetingSession::onParticipantsUpdated(livekit::Room &room, const livekit::ParticipantsUpdatedEvent &ev) {
    Q_UNUSED(room);
    for (const auto &participant : ev.participants) {
        if (participant) {
            const QString participantId = QString::fromStdString(participant->identity());
            const QString name = QString::fromStdString(participant->name());
            qDebug() << QThread::currentThread() << __FUNCTION__ << "participant id=" << participantId << "name=" << name;
        }
    }
}

void MeetingSession::onParticipantDisconnected(livekit::Room &room, const livekit::ParticipantDisconnectedEvent &ev) {
    Q_UNUSED(room);
    const QString participantId = ev.participant ? QString::fromStdString(ev.participant->identity()) : QString();
    const QString name = ev.participant ? QString::fromStdString(ev.participant->name()) : QString();
    qDebug() << QThread::currentThread() << __FUNCTION__ << "participant disconnected: id=" << participantId
        << "name=" << name << "reason=" << disconnectReasonToString(ev.reason);
    if (remoteParticipants_.remove(participantId)) {
        qDebug() << QThread::currentThread() << __FUNCTION__ << "participant with id" << participantId << "removed from remoteParticipants";
    }

    emit sigParticipantLeft(participantId);
}

void MeetingSession::onLocalTrackPublished(livekit::Room &room, const livekit::LocalTrackPublishedEvent &ev) {
    Q_UNUSED(room);
    const QString trackSid = ev.publication ? QString::fromStdString(ev.publication->sid()) : QString();
    const QString trackName = ev.publication ? QString::fromStdString(ev.publication->name()) : QString();
    qDebug() << QThread::currentThread() << __FUNCTION__ << "local track published: track_sid=" << trackSid
        << "name=" << trackName
        << "kind=" << (ev.track ? trackKindToString(ev.track->kind()) : QString())
        << "source=" << (ev.publication ? trackSourceToString(ev.publication->source()) : QString());
}

void MeetingSession::onLocalTrackUnpublished(livekit::Room &room,
    const livekit::LocalTrackUnpublishedEvent &ev) {
    Q_UNUSED(room);
    const QString trackSid = ev.publication ? QString::fromStdString(ev.publication->sid()) : QString();
    const QString trackName = ev.publication ? QString::fromStdString(ev.publication->name()) : QString();
    qDebug() << QThread::currentThread() << __FUNCTION__ << "local track unpublished: track_sid=" << trackSid
        << "name=" << trackName
        << "source=" << (ev.publication ? trackSourceToString(ev.publication->source()) : QString());
}

void MeetingSession::onLocalTrackSubscribed(livekit::Room &room, const livekit::LocalTrackSubscribedEvent &ev) {
    Q_UNUSED(room);
    const QString trackSid = ev.track ? QString::fromStdString(ev.track->sid()) : QString();
    const QString trackName = ev.track ? QString::fromStdString(ev.track->name()) : QString();
    qDebug() << QThread::currentThread() << __FUNCTION__ << "local track subscribed: track_sid=" << trackSid
        << "name=" << trackName
        << "kind=" << (ev.track ? trackKindToString(ev.track->kind()) : QString());
}

void MeetingSession::onTrackPublished(livekit::Room &room, const livekit::TrackPublishedEvent &ev) {
    Q_UNUSED(room);
    const QString participantId = ev.participant ? QString::fromStdString(ev.participant->identity()) : QString();
    const QString trackSid = ev.publication ? QString::fromStdString(ev.publication->sid()) : QString();
    const QString trackName = ev.publication ? QString::fromStdString(ev.publication->name()) : QString();
    qDebug() << QThread::currentThread() << __FUNCTION__ << "track published: participant_id=" << participantId
        << "track_sid=" << trackSid
        << "name=" << trackName
        << "kind=" << (ev.publication ? trackKindToString(ev.publication->kind()) : QString())
        << "source=" << (ev.publication ? trackSourceToString(ev.publication->source()) : QString());
}

void MeetingSession::onTrackUnpublished(livekit::Room &room, const livekit::TrackUnpublishedEvent &ev) {
    Q_UNUSED(room);
    const QString participantId = ev.participant ? QString::fromStdString(ev.participant->identity()) : QString();
    const QString trackSid = ev.publication ? QString::fromStdString(ev.publication->sid()) : QString();
    const QString trackName = ev.publication ? QString::fromStdString(ev.publication->name()) : QString();
    qDebug() << QThread::currentThread() << __FUNCTION__ << "track unpublished: participant_id=" << participantId
        << "track_sid=" << trackSid
        << "name=" << trackName;
}

void MeetingSession::onTrackSubscribed(livekit::Room &room, const livekit::TrackSubscribedEvent &ev) {
    Q_UNUSED(room);
    const QString participantId = ev.participant ? QString::fromStdString(ev.participant->identity()) : QString();
    const QString trackSid = ev.publication ? QString::fromStdString(ev.publication->sid()) : QString();
    const QString trackName = ev.publication ? QString::fromStdString(ev.publication->name()) : QString();
    const QString trackKind = ev.publication ? trackKindToString(ev.publication->kind()) : QString();
    const QString trackSource = ev.publication ? trackSourceToString(ev.publication->source()) : QString();
    qDebug() << QThread::currentThread() << __FUNCTION__ << "track subscribed: participant_id=" << participantId
        << "track_sid=" << trackSid
        << "name=" << trackName
        << "kind=" << trackKind
        << "source=" << trackSource;

    startRemoteTrackMedia(ev.track);

    if (ev.track) {
        if (remoteParticipants_.contains(participantId)) {
            if (ev.track->kind() == livekit::TrackKind::KIND_AUDIO) {
                remoteParticipants_[participantId].setAudioTrackSid(trackSid);
            } else if (ev.track->kind() == livekit::TrackKind::KIND_VIDEO) {
                remoteParticipants_[participantId].setVideoTrackSid(trackSid);
            }
        } else {
            MeetingParticipant participant(ev.participant, false);
            if (ev.track->kind() == livekit::TrackKind::KIND_AUDIO) {
                participant.setAudioTrackSid(trackSid);
            } else if (ev.track->kind() == livekit::TrackKind::KIND_VIDEO) {
                participant.setVideoTrackSid(trackSid);
            }
            remoteParticipants_.emplace(participantId, std::move(participant));
        }
        emit sigTrackSubscribed(participantId, static_cast<int>(ev.track->kind()));
    }
}

void MeetingSession::onTrackUnsubscribed(livekit::Room &room, const livekit::TrackUnsubscribedEvent &ev) {
    Q_UNUSED(room);
    const QString participantId = ev.participant ? QString::fromStdString(ev.participant->identity()) : QString();
    const QString trackSid = ev.publication ? QString::fromStdString(ev.publication->sid()) : QString();
    const QString trackName = ev.publication ? QString::fromStdString(ev.publication->name()) : QString();
    qDebug() << QThread::currentThread() << __FUNCTION__ << "track unsubscribed: participant_id=" << participantId
        << "track_sid=" << trackSid
        << "name=" << trackName;

    if (ev.track) {
        if (remoteParticipants_.contains(participantId)) {
            if (ev.track->kind() == livekit::TrackKind::KIND_AUDIO) {
                remoteParticipants_[participantId].setAudioTrackSid(QString());
            } else if (ev.track->kind() == livekit::TrackKind::KIND_VIDEO) {
                remoteParticipants_[participantId].setVideoTrackSid(QString());
            }
        }
        emit sigTrackUnsubscribed(participantId, static_cast<int>(ev.track->kind()));
    }

    stopRemoteTrackMedia(ev.track);
}

void MeetingSession::onTrackSubscriptionFailed(livekit::Room &room,
    const livekit::TrackSubscriptionFailedEvent &ev) {
    Q_UNUSED(room);
    const QString participantId = ev.participant ? QString::fromStdString(ev.participant->identity()) : QString();
    const QString trackSid = QString::fromStdString(ev.track_sid);
    const QString errorMsg = QString::fromStdString(ev.error);
    qDebug() << QThread::currentThread() << __FUNCTION__ << "track subscription failed: participant_id=" << participantId
        << "track_sid=" << trackSid
        << "error=" << errorMsg;
}

void MeetingSession::onTrackMuted(livekit::Room &room, const livekit::TrackMutedEvent &ev) {
    Q_UNUSED(room);
    const QString participantId = ev.participant ? QString::fromStdString(ev.participant->identity()) : QString();
    const QString trackSid = ev.publication ? QString::fromStdString(ev.publication->sid()) : QString();
    const QString trackName = ev.publication ? QString::fromStdString(ev.publication->name()) : QString();
    qDebug() << QThread::currentThread() << __FUNCTION__ << "track muted: participant_id=" << participantId
        << "track_sid=" << trackSid
        << "name=" << trackName;
}

void MeetingSession::onTrackUnmuted(livekit::Room &room, const livekit::TrackUnmutedEvent &ev) {
    Q_UNUSED(room);
    const QString participantId = ev.participant ? QString::fromStdString(ev.participant->identity()) : QString();
    const QString trackSid = ev.publication ? QString::fromStdString(ev.publication->sid()) : QString();
    const QString trackName = ev.publication ? QString::fromStdString(ev.publication->name()) : QString();
    qDebug() << QThread::currentThread() << __FUNCTION__ << "track unmuted: participant_id=" << participantId
        << "track_sid=" << trackSid
        << "name=" << trackName;
}

void MeetingSession::onActiveSpeakersChanged(livekit::Room &room,
    const livekit::ActiveSpeakersChangedEvent &ev) {
    Q_UNUSED(room);
    QStringList activeSpeakerIds;
    for (const auto &participant : ev.speakers) {
        if (participant) {
            activeSpeakerIds << QString::fromStdString(participant->identity());
        }
    }
    qDebug() << QThread::currentThread() << __FUNCTION__ << "active speakers changed:" << activeSpeakerIds.join(", ");
}

void MeetingSession::onRoomMetadataChanged(livekit::Room &room, const livekit::RoomMetadataChangedEvent &ev) {
    Q_UNUSED(room);
    const QString oldMetadata = QString::fromStdString(ev.old_metadata);
    const QString newMetadata = QString::fromStdString(ev.new_metadata);
    qDebug() << QThread::currentThread() << __FUNCTION__ << "room metadata changed: old=" << oldMetadata << "new=" << newMetadata;
}

void MeetingSession::onRoomSidChanged(livekit::Room &room, const livekit::RoomSidChangedEvent &ev) {
    Q_UNUSED(room);
    const QString newSid = QString::fromStdString(ev.sid);
    qDebug() << QThread::currentThread() << __FUNCTION__ << "room SID changed: new SID=" << newSid;
}

void MeetingSession::onRoomUpdated(livekit::Room &room, const livekit::RoomUpdatedEvent &ev) {
    Q_UNUSED(room);
    qDebug() << QThread::currentThread() << __FUNCTION__ << "room updated";
    logRoomSnapshot(__FUNCTION__, ev.info);
}

void MeetingSession::onRoomMoved(livekit::Room &room, const livekit::RoomMovedEvent &ev) {
    Q_UNUSED(room);
    qDebug() << QThread::currentThread() << __FUNCTION__ << "room moved";
    logRoomSnapshot(__FUNCTION__, ev.info);
}

void MeetingSession::onParticipantMetadataChanged(livekit::Room &room,
    const livekit::ParticipantMetadataChangedEvent &ev) {
    Q_UNUSED(room);
    const QString participantId = ev.participant ? QString::fromStdString(ev.participant->identity()) : QString();
    const QString oldMetadata = QString::fromStdString(ev.old_metadata);
    const QString newMetadata = QString::fromStdString(ev.new_metadata);
    qDebug() << QThread::currentThread() << __FUNCTION__ << "participant metadata changed: participant_id=" << participantId
        << "old=" << oldMetadata
        << "new=" << newMetadata;
}

void MeetingSession::onParticipantAttributesChanged(livekit::Room &room,
    const livekit::ParticipantAttributesChangedEvent &ev) {
    Q_UNUSED(room);
    const QString participantId = ev.participant ? QString::fromStdString(ev.participant->identity()) : QString();
    qDebug() << QThread::currentThread() << __FUNCTION__ << "participant attributes changed: participant_id=" << participantId
        << "changed attributes:";
    for (const auto &attr : ev.changed_attributes) {
        qDebug() << "    " << QString::fromStdString(attr.key) << "=" << QString::fromStdString(attr.value);
    }
}

void MeetingSession::onParticipantEncryptionStatusChanged(livekit::Room &room,
    const livekit::ParticipantEncryptionStatusChangedEvent &ev) {
    Q_UNUSED(room);
    const QString participantId = ev.participant ? QString::fromStdString(ev.participant->identity()) : QString();
    qDebug() << QThread::currentThread() << __FUNCTION__ << "participant encryption status changed: participant_id=" << participantId 
        << "encryption enabled:" << (ev.is_encrypted ? "yes" : "no");
}

void MeetingSession::onConnectionQualityChanged(livekit::Room &room,
    const livekit::ConnectionQualityChangedEvent &ev) {
    Q_UNUSED(room);
    const QString participantId = ev.participant ? QString::fromStdString(ev.participant->identity()) : QString();
    qDebug() << QThread::currentThread() << __FUNCTION__ << "connection quality changed: participant_id=" << participantId
        << "quality=" << connectionQualityToString(ev.quality);
}

void MeetingSession::onConnectionStateChanged(livekit::Room &room,
    const livekit::ConnectionStateChangedEvent &ev) {
    Q_UNUSED(room);
    qDebug() << QThread::currentThread() << __FUNCTION__ << "room connection state changed: state=" << connectionStateToString(ev.state);
}

void MeetingSession::onDisconnected(livekit::Room &room, const livekit::DisconnectedEvent &ev) {
    Q_UNUSED(room);
    qDebug() << QThread::currentThread() << __FUNCTION__ << "room disconnected from room: reason=" << disconnectReasonToString(ev.reason);
    setRoomState(MeetingSessionRoomState::Disconnected);
}

void MeetingSession::onReconnecting(livekit::Room &room, const livekit::ReconnectingEvent &ev) {
    Q_UNUSED(room);
    Q_UNUSED(ev);
    qDebug() << QThread::currentThread() << __FUNCTION__ << "reconnecting to room";
    setRoomState(MeetingSessionRoomState::Reconnecting);
}

void MeetingSession::onReconnected(livekit::Room &room, const livekit::ReconnectedEvent &ev) {
    Q_UNUSED(room);
    Q_UNUSED(ev);
    qDebug() << QThread::currentThread() << __FUNCTION__ << "reconnected to room";
    setRoomState(MeetingSessionRoomState::Connected);
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

void MeetingSession::setScreenShareState(MeetingSessionMediaState state) {
    if (screenShareState_ == state) {
        return;
    }

    screenShareState_ = state;
    emit sigScreenShareStateChanged(screenShareState_);
}

void MeetingSession::setRecordingState(MeetingSessionMediaState state) {
    if (recordingState_ == state) {
        return;
    }

    recordingState_ = state;
    emit sigRecordingStateChanged(recordingState_);
}