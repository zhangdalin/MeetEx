#include "meeting_session.h"

#include "media_engine.h"
#include "media_util.h"

#include "livekit/local_participant.h"
#include "livekit/remote_participant.h"

#include <QDebug>

namespace {

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

}

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
    , context_(context) {
    MediaEngine::instance().init();
    room_.setDelegate(this);
}

MeetingSession::~MeetingSession() {
    shutdown();
    room_.setDelegate(nullptr);
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
        || !localAudioTrackSid_.isEmpty()
        || !localVideoTrackSid_.isEmpty();

    if (!hasActiveRoom && !hasLocalMedia) {
        return;
    }

    if (hasActiveRoom) {
        setRoomState(MeetingSessionRoomState::Disconnecting);
    }

    stopLocalMediaCapture(hasActiveRoom);
    disconnectRoom();
    clearSessionCaches();
}

bool MeetingSession::startAudio() {
    auto *localParticipant = getLocalParticipant();
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
        localAudioTrackSid_.clear();
        return false;
    }
    localAudioTrackSid_ = QString::fromStdString(localAudioSidStd);

    setMicrophoneState(MeetingSessionMediaState::On);
    return true;
}

void MeetingSession::stopAudio() {
    if (microphoneState_ == MeetingSessionMediaState::Off && localAudioTrackSid_.isEmpty()) {
        return;
    }

    livekit::LocalParticipant *localParticipant = nullptr;
    if (roomState_ == MeetingSessionRoomState::Connected) {
        localParticipant = getLocalParticipant();
    }

    setMicrophoneState(MeetingSessionMediaState::Stopping);
    MediaEngine::instance().stopLocalAudio(localParticipant, localAudioTrackSid_.toStdString());
    localAudioTrackSid_.clear();
    setMicrophoneState(MeetingSessionMediaState::Off);
}

bool MeetingSession::startVideo() {
    auto *localParticipant = getLocalParticipant();
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
        localVideoTrackSid_.clear();
        return false;
    }
    localVideoTrackSid_ = QString::fromStdString(localVideoSidStd);

    setCameraState(MeetingSessionMediaState::On);
    return true;
}

void MeetingSession::stopVideo() {
    if (cameraState_ == MeetingSessionMediaState::Off && localVideoTrackSid_.isEmpty()) {
        return;
    }

    livekit::LocalParticipant *localParticipant = nullptr;
    if (roomState_ == MeetingSessionRoomState::Connected) {
        localParticipant = getLocalParticipant();
    }

    setCameraState(MeetingSessionMediaState::Stopping);
    MediaEngine::instance().stopLocalVideo(localParticipant, localVideoTrackSid_.toStdString());
    localVideoTrackSid_.clear();
    setCameraState(MeetingSessionMediaState::Off);
}

QString MeetingSession::localParticipantId() const {
    if (!localParticipant_) {
        return QString();
    }
    return QString::fromStdString(localParticipant_->identity());
}

QString MeetingSession::localParticipantName() const {
    if (!localParticipant_) {
        return QString();
    }
    return QString::fromStdString(localParticipant_->name());
}

QString MeetingSession::localVideoTrackSid() const {
    return localVideoTrackSid_;
}

AudioLevelInfo MeetingSession::localAudioLevel() const {
    return MediaEngine::instance().localAudioLevel();
}

bool MeetingSession::isLocalAudioSpeaking() const {
    return MediaEngine::instance().isLocalAudioSpeaking();
}

QHash<QString, AudioLevelInfo> MeetingSession::remoteAudioLevels() const {
    const auto stdMap = MediaEngine::instance().remoteAudioLevels();
    QHash<QString, AudioLevelInfo> qtMap;
    for (const auto &pair : stdMap) {
        qtMap.insert(QString::fromStdString(pair.first), pair.second);
    }
    return qtMap;
}

QVector<MeetingSessionRemoteParticipantInfo> MeetingSession::remoteUsers() const {
    QVector<MeetingSessionRemoteParticipantInfo> remoteUsers;
    if (roomState_ != MeetingSessionRoomState::Connected) {
        return remoteUsers;
    }

    const auto remoteParticipants = room_.remoteParticipants();
    remoteUsers.reserve(static_cast<qsizetype>(remoteParticipants.size()));
    for (const auto &participant : remoteParticipants) {
        if (participant) {
            remoteUsers.append(buildRemoteParticipantInfo(participant.get()));
        }
    }
    return remoteUsers;
}

QString MeetingSession::getParticipantDisplayName(const QString &participantId, const QString &participantName)
{
    const QString trimmedName = participantName.trimmed();

    // Check if we already have a cached name for this participant
    const auto it = participantDisplayNames_.find(participantId);
    if (it != participantDisplayNames_.end()) {
        // Prefer the latest non-empty participant name over generated Guest names.
        if (!trimmedName.isEmpty() && it.value() != trimmedName) {
            participantDisplayNames_[participantId] = trimmedName;
            return trimmedName;
        }
        return it.value();
    }

    // No cache yet - use provided name if available, otherwise generate Guest name
    if (!trimmedName.isEmpty()) {
        participantDisplayNames_[participantId] = trimmedName;
        return trimmedName;
    }

    // Generate a Guest name for this participant
    const QString guestName = QStringLiteral("Guest%1").arg(nextGuestIndex_++);
    participantDisplayNames_[participantId] = guestName;
    return guestName;
}

QString MeetingSession::getParticipantIdByTrackSid(const QString &trackSid) const
{
    const auto it = trackToParticipantMap_.find(trackSid);
    return it != trackToParticipantMap_.end() ? it.value() : QString();
}

void MeetingSession::mapTrackToParticipant(const QString &trackSid, const QString &participantId)
{
    trackToParticipantMap_[trackSid] = participantId;
}

void MeetingSession::unmapTrack(const QString &trackSid)
{
    trackToParticipantMap_.remove(trackSid);
}

void MeetingSession::clearParticipantData(const QString &participantId)
{
    // Remove participant display name from cache
    participantDisplayNames_.remove(participantId);

    // Remove all track mappings for this participant
    for (auto it = trackToParticipantMap_.begin(); it != trackToParticipantMap_.end();) {
        if (it.value() == participantId) {
            it = trackToParticipantMap_.erase(it);
        } else {
            ++it;
        }
    }
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

    qInfo() << __FUNCTION__ << "set room option with Url:" << context_.livekitUrl << "\n"
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
    qInfo() << __FUNCTION__ << "Connect result is" << (res ? "successful" : "failed");
    if (!res) {
        setRoomState(MeetingSessionRoomState::Disconnected);
        qCritical() << __FUNCTION__ << "Failed to connect to room";
        livekit::shutdown();
        return false;
    }

    const auto info = room_.room_info();
    qInfo() << __FUNCTION__ << "Connected to room:"
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

bool MeetingSession::disconnectRoom() {
    MediaEngine::instance().stopAllAudioPlay();
    MediaEngine::instance().stopAllVideoRender();

    if (roomState_ == MeetingSessionRoomState::Disconnected) {
        qWarning() << __FUNCTION__ << "called but already disconnected";
        return true;
    }

    room_.setDelegate(nullptr);
    qInfo() << __FUNCTION__ << "Disconnected from room";
    localParticipant_ = nullptr;
    localAudioTrackSid_.clear();
    localVideoTrackSid_.clear();
    setRoomState(MeetingSessionRoomState::Disconnected);

    return true;
}

livekit::LocalParticipant *MeetingSession::getLocalParticipant() const {
    if (roomState_ != MeetingSessionRoomState::Connected) {
        qWarning() << __FUNCTION__ << "called but not connected to room";
        return nullptr;
    }
    if (!localParticipant_) {
        auto *localParticipant = room_.localParticipant();
        if (!localParticipant) {
            qWarning() << __FUNCTION__ << "room has no local participant";
            return nullptr;
        }
        const_cast<MeetingSession *>(this)->localParticipant_ = localParticipant;
    }
    return localParticipant_;
}

void MeetingSession::resetSessionMediaState() {
    localParticipant_ = nullptr;
    localAudioTrackSid_.clear();
    localVideoTrackSid_.clear();
    setMicrophoneState(MeetingSessionMediaState::Off);
    setCameraState(MeetingSessionMediaState::Off);
}

void MeetingSession::stopLocalMediaCapture(bool unpublishTracks) {
    if (cameraState_ != MeetingSessionMediaState::Off || !localVideoTrackSid_.isEmpty()) {
        setCameraState(MeetingSessionMediaState::Stopping);
        MediaEngine::instance().stopLocalVideo(unpublishTracks ? getLocalParticipant() : nullptr,
            localVideoTrackSid_.toStdString());
        localVideoTrackSid_.clear();
        setCameraState(MeetingSessionMediaState::Off);
    }

    if (microphoneState_ != MeetingSessionMediaState::Off || !localAudioTrackSid_.isEmpty()) {
        setMicrophoneState(MeetingSessionMediaState::Stopping);
        MediaEngine::instance().stopLocalAudio(unpublishTracks ? getLocalParticipant() : nullptr,
            localAudioTrackSid_.toStdString());
        localAudioTrackSid_.clear();
        setMicrophoneState(MeetingSessionMediaState::Off);
    }
}

bool MeetingSession::shouldStartMicrophoneOnJoin() const {
    return context_.joinOptions.autoConnectAudio && context_.joinOptions.startMicrophone;
}

bool MeetingSession::shouldStartCameraOnJoin() const {
    return context_.joinOptions.startCamera;
}

void MeetingSession::clearSessionCaches() {
    participantDisplayNames_.clear();
    trackToParticipantMap_.clear();
}

MeetingSession::ParticipantEventInfo MeetingSession::buildParticipantEventInfo(
    livekit::Participant *participant) const {
    return {
        participant ? QString::fromStdString(participant->identity()) : QString(),
        participant ? QString::fromStdString(participant->name()) : QString()
    };
}

MeetingSession::TrackEventInfo MeetingSession::buildTrackEventInfo(
    livekit::Participant *participant, livekit::TrackPublication *publication) const {
    const auto participantInfo = buildParticipantEventInfo(participant);
    return {
        participantInfo.participantId,
        publication ? QString::fromStdString(publication->sid()) : QString(),
        publication ? QString::fromStdString(publication->name()) : QString()
    };
}

MeetingSessionRemoteParticipantInfo MeetingSession::buildRemoteParticipantInfo(
    const livekit::RemoteParticipant *participant) const {
    return {
        QString::fromStdString(participant->identity()),
        QString::fromStdString(participant->name()),
        QString::fromStdString(participant->metadata())
    };
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

void MeetingSession::startRemoteTrackMedia(const std::shared_ptr<livekit::Track> &track,
    const QString &trackSid) const {
    if (!track) {
        return;
    }

    if (track->kind() == livekit::TrackKind::KIND_VIDEO) {
        livekit::VideoStream::Options opts;
        opts.format = livekit::VideoBufferType::RGBA;
        auto videoStream = livekit::VideoStream::fromTrack(track, opts);
        if (!videoStream) {
            qCritical() << __FUNCTION__ << "Failed to create VideoStream for track" << trackSid;
            return;
        }
        if (!MediaEngine::instance().startVideoRender(videoStream, trackSid.toStdString())) {
            qCritical() << __FUNCTION__ << "video render failed for track" << trackSid;
        }
        return;
    }

    if (track->kind() == livekit::TrackKind::KIND_AUDIO) {
        livekit::AudioStream::Options opts;
        auto audioStream = livekit::AudioStream::fromTrack(track, opts);
        if (!audioStream) {
            qCritical() << __FUNCTION__ << "Failed to create AudioStream for track" << trackSid;
            return;
        }
        if (!MediaEngine::instance().startAudioPlay(audioStream, trackSid.toStdString())) {
            qCritical() << __FUNCTION__ << "audio play failed for track" << trackSid;
        }
    }
}

void MeetingSession::stopRemoteTrackMedia(const std::shared_ptr<livekit::Track> &track,
    const QString &trackSid) const {
    if (!track) {
        return;
    }

    if (track->kind() == livekit::TrackKind::KIND_VIDEO) {
        MediaEngine::instance().stopVideoRender(trackSid.toStdString());
        return;
    }

    if (track->kind() == livekit::TrackKind::KIND_AUDIO) {
        MediaEngine::instance().stopAudioPlay(trackSid.toStdString());
    }
}

void MeetingSession::onParticipantConnected(livekit::Room &room, const livekit::ParticipantConnectedEvent &ev) {
    Q_UNUSED(room);
    if (!ev.participant) {
        qWarning() << __FUNCTION__ << "participant connected event without participant";
        return;
    }

    const auto participantInfo = buildParticipantEventInfo(ev.participant);
    qDebug() << __FUNCTION__ << "participant connected: id=" << participantInfo.participantId
        << "name=" << participantInfo.participantName;
    emit sigParticipantJoined(participantInfo.participantId, participantInfo.participantName);
}

void MeetingSession::onParticipantsUpdated(livekit::Room &room, const livekit::ParticipantsUpdatedEvent &ev) {
    Q_UNUSED(room);
    const QString localId = localParticipantId();
    for (const auto &participant : ev.participants) {
        if (participant) {
            const QString participantId = QString::fromStdString(participant->identity());
            const QString participantName = QString::fromStdString(participant->name());
            qDebug() << __FUNCTION__ << "participant id="
                << participantId
                << "name=" << participantName;

            if (!participantId.isEmpty() && participantId != localId) {
                const QString resolvedName = getParticipantDisplayName(participantId, participantName);
                emit sigParticipantJoined(participantId, resolvedName);
            }
        }
    }
}

void MeetingSession::onParticipantDisconnected(livekit::Room &room,
    const livekit::ParticipantDisconnectedEvent &ev) {
    Q_UNUSED(room);
    const auto participantInfo = buildParticipantEventInfo(ev.participant);
    qDebug() << __FUNCTION__ << "participant disconnected: id=" << participantInfo.participantId
        << "name=" << participantInfo.participantName << "reason=" << disconnectReasonToString(ev.reason);

    clearParticipantData(participantInfo.participantId);
    emit sigParticipantLeft(participantInfo.participantId, participantInfo.participantName);
}

void MeetingSession::onLocalTrackPublished(livekit::Room &room, const livekit::LocalTrackPublishedEvent &ev) {
    Q_UNUSED(room);
    const QString trackSid = ev.publication ? QString::fromStdString(ev.publication->sid()) : QString();
    const QString trackName = ev.publication ? QString::fromStdString(ev.publication->name()) : QString();
    qDebug() << __FUNCTION__ << "local track published: track_sid=" << trackSid
        << "name=" << trackName
        << "kind=" << (ev.track ? trackKindToString(ev.track->kind()) : "")
        << "source=" << (ev.publication ? trackSourceToString(ev.publication->source()) : "");
}

void MeetingSession::onLocalTrackUnpublished(livekit::Room &room,
    const livekit::LocalTrackUnpublishedEvent &ev) {
    Q_UNUSED(room);
    const QString trackSid = ev.publication ? QString::fromStdString(ev.publication->sid()) : QString();
    const QString trackName = ev.publication ? QString::fromStdString(ev.publication->name()) : QString();
    qDebug() << __FUNCTION__ << "local track unpublished: track_sid=" << trackSid << "name=" << trackName;
}

void MeetingSession::onLocalTrackSubscribed(livekit::Room &room, const livekit::LocalTrackSubscribedEvent &ev) {
    Q_UNUSED(room);
    const QString trackSid = ev.track ? QString::fromStdString(ev.track->sid()) : QString();
    const QString trackName = ev.track ? QString::fromStdString(ev.track->name()) : QString();
    qDebug() << __FUNCTION__ << "local track subscribed: track_sid=" << trackSid
        << "name=" << trackName
        << "kind=" << (ev.track ? trackKindToString(ev.track->kind()) : "");
}

void MeetingSession::onTrackPublished(livekit::Room &room, const livekit::TrackPublishedEvent &ev) {
    Q_UNUSED(room);
    const auto eventInfo = buildTrackEventInfo(ev.participant, ev.publication.get());
    qDebug() << __FUNCTION__ << "track published: participant_id=" << eventInfo.participantId
        << "track_sid=" << eventInfo.trackSid
        << "name=" << eventInfo.trackName
        << "kind=" << (ev.publication ? trackKindToString(ev.publication->kind()) : "")
        << "source=" << (ev.publication ? trackSourceToString(ev.publication->source()) : "");
}

void MeetingSession::onTrackUnpublished(livekit::Room &room, const livekit::TrackUnpublishedEvent &ev) {
    Q_UNUSED(room);
    const auto eventInfo = buildTrackEventInfo(ev.participant, ev.publication.get());
    qDebug() << __FUNCTION__ << "track unpublished: participant_id=" << eventInfo.participantId
        << "track_sid=" << eventInfo.trackSid << "name=" << eventInfo.trackName;
}

void MeetingSession::onTrackSubscribed(livekit::Room &room, const livekit::TrackSubscribedEvent &ev) {
    Q_UNUSED(room);
    const auto eventInfo = buildTrackEventInfo(ev.participant, ev.publication.get());
    qDebug() << __FUNCTION__ << "track subscribed: participant_id=" << eventInfo.participantId
        << "track_sid=" << eventInfo.trackSid
        << "name=" << eventInfo.trackName
        << "kind=" << (ev.track ? trackKindToString(ev.track->kind()) : "")
        << "source=" << (ev.publication ? trackSourceToString(ev.publication->source()) : "");

    if (!eventInfo.trackSid.isEmpty() && !eventInfo.participantId.isEmpty()) {
        mapTrackToParticipant(eventInfo.trackSid, eventInfo.participantId);
    }

    if (ev.track) {
        const int trackKind = static_cast<int>(ev.track->kind());
        emit sigTrackSubscribed(eventInfo.trackSid, eventInfo.trackName, eventInfo.participantId, trackKind);
    }

    startRemoteTrackMedia(ev.track, eventInfo.trackSid);
}

void MeetingSession::onTrackUnsubscribed(livekit::Room &room, const livekit::TrackUnsubscribedEvent &ev) {
    Q_UNUSED(room);
    const auto eventInfo = buildTrackEventInfo(ev.participant, ev.publication.get());
    qDebug() << __FUNCTION__ << "track unsubscribed: participant_id=" << eventInfo.participantId
        << "track_sid=" << eventInfo.trackSid << "name=" << eventInfo.trackName;

    if (!eventInfo.trackSid.isEmpty()) {
        unmapTrack(eventInfo.trackSid);
    }

    if (ev.track) {
        const int trackKind = static_cast<int>(ev.track->kind());
        emit sigTrackUnsubscribed(eventInfo.trackSid, eventInfo.trackName, eventInfo.participantId, trackKind);
    }

    stopRemoteTrackMedia(ev.track, eventInfo.trackSid);
}

void MeetingSession::onTrackSubscriptionFailed(livekit::Room &room,
    const livekit::TrackSubscriptionFailedEvent &ev) {
    Q_UNUSED(room);
    const QString participantId = ev.participant ? QString::fromStdString(ev.participant->identity()) : QString();
    const QString trackSid = QString::fromStdString(ev.track_sid);
    const QString errorMsg = QString::fromStdString(ev.error);
    qDebug() << __FUNCTION__ << "track subscription failed: participant_id=" << participantId
        << "track_sid=" << trackSid << "error=" << errorMsg;
}

void MeetingSession::onTrackMuted(livekit::Room &room, const livekit::TrackMutedEvent &ev) {
    Q_UNUSED(room);
    const auto eventInfo = buildTrackEventInfo(ev.participant, ev.publication.get());
    qDebug() << __FUNCTION__ << "track muted: participant_id=" << eventInfo.participantId
        << "track_sid=" << eventInfo.trackSid << "name=" << eventInfo.trackName;
}

void MeetingSession::onTrackUnmuted(livekit::Room &room, const livekit::TrackUnmutedEvent &ev) {
    Q_UNUSED(room);
    const auto eventInfo = buildTrackEventInfo(ev.participant, ev.publication.get());
    qDebug() << __FUNCTION__ << "track unmuted: participant_id=" << eventInfo.participantId
        << "track_sid=" << eventInfo.trackSid << "name=" << eventInfo.trackName;
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
    qDebug() << __FUNCTION__ << "active speakers changed:" << activeSpeakerIds.join(", ");
}

void MeetingSession::onRoomMetadataChanged(livekit::Room &room, const livekit::RoomMetadataChangedEvent &ev) {
    Q_UNUSED(room);
    const QString oldMetadata = QString::fromStdString(ev.old_metadata);
    const QString newMetadata = QString::fromStdString(ev.new_metadata);
    qDebug() << __FUNCTION__ << "room metadata changed: old=" << oldMetadata << "new=" << newMetadata;
}

void MeetingSession::onRoomSidChanged(livekit::Room &room, const livekit::RoomSidChangedEvent &ev) {
    Q_UNUSED(room);
    const QString newSid = QString::fromStdString(ev.sid);
    qDebug() << __FUNCTION__ << "room SID changed: new SID=" << newSid;
}

void MeetingSession::onRoomUpdated(livekit::Room &room, const livekit::RoomUpdatedEvent &ev) {
    Q_UNUSED(room);
    logRoomSnapshot(__FUNCTION__, ev.info);
}

void MeetingSession::onRoomMoved(livekit::Room &room, const livekit::RoomMovedEvent &ev) {
    Q_UNUSED(room);
    logRoomSnapshot(__FUNCTION__, ev.info);
}

void MeetingSession::onParticipantMetadataChanged(livekit::Room &room,
    const livekit::ParticipantMetadataChangedEvent &ev) {
    Q_UNUSED(room);
    const auto participantInfo = buildParticipantEventInfo(ev.participant);
    const QString oldMetadata = QString::fromStdString(ev.old_metadata);
    const QString newMetadata = QString::fromStdString(ev.new_metadata);
    qDebug() << __FUNCTION__ << "participant metadata changed: participant_id=" << participantInfo.participantId
        << "old=" << oldMetadata << "new=" << newMetadata;
}

void MeetingSession::onParticipantAttributesChanged(livekit::Room &room,
    const livekit::ParticipantAttributesChangedEvent &ev) {
    Q_UNUSED(room);
    const auto participantInfo = buildParticipantEventInfo(ev.participant);
    QVector<livekit::AttributeEntry> qtAttributes(
        ev.changed_attributes.begin(), ev.changed_attributes.end());
    const QStringList changedAttributes = buildChangedAttributes(qtAttributes);
    qDebug() << __FUNCTION__ << "participant attributes changed: participant_id=" << participantInfo.participantId
        << "changed attributes:" << changedAttributes.join(", ");
}

void MeetingSession::onParticipantEncryptionStatusChanged(livekit::Room &room,
    const livekit::ParticipantEncryptionStatusChangedEvent &ev) {
    Q_UNUSED(room);
    const auto participantInfo = buildParticipantEventInfo(ev.participant);
    qDebug() << __FUNCTION__ << "participant encryption status changed: participant_id="
        << participantInfo.participantId << "encryption enabled:" << (ev.is_encrypted ? "yes" : "no");
}

void MeetingSession::onConnectionQualityChanged(livekit::Room &room,
    const livekit::ConnectionQualityChangedEvent &ev) {
    Q_UNUSED(room);
    const auto participantInfo = buildParticipantEventInfo(ev.participant);
    qDebug() << __FUNCTION__ << "connection quality changed: participant_id="
        << participantInfo.participantId << "quality=" << connectionQualityToString(ev.quality);
}

void MeetingSession::onConnectionStateChanged(livekit::Room &room,
    const livekit::ConnectionStateChangedEvent &ev) {
    Q_UNUSED(room);
    qDebug() << __FUNCTION__ << "connection state changed: state=" << connectionStateToString(ev.state);
}

void MeetingSession::onDisconnected(livekit::Room &room, const livekit::DisconnectedEvent &ev) {
    Q_UNUSED(room);
    qDebug() << __FUNCTION__ << "disconnected from room: reason=" << disconnectReasonToString(ev.reason);

    stopLocalMediaCapture(false);
    MediaEngine::instance().stopAllAudioPlay();
    MediaEngine::instance().stopAllVideoRender();
    resetSessionMediaState();
    setRoomState(MeetingSessionRoomState::Disconnected);
}

void MeetingSession::onReconnecting(livekit::Room &room, const livekit::ReconnectingEvent &ev) {
    Q_UNUSED(room);
    Q_UNUSED(ev);
    qDebug() << __FUNCTION__ << "reconnecting to room";
    setRoomState(MeetingSessionRoomState::Reconnecting);
}

void MeetingSession::onReconnected(livekit::Room &room, const livekit::ReconnectedEvent &ev) {
    Q_UNUSED(room);
    Q_UNUSED(ev);
    qDebug() << __FUNCTION__ << "reconnected to room";
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