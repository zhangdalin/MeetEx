#ifndef MEETING_SESSION_H
#define MEETING_SESSION_H

#include "meeting_def.h"
#include "media_mgr.h"

#include "livekit/room.h"
#include "livekit/room_delegate.h"

#include <QObject>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QVector>
#include <memory>
#include <string>

namespace livekit {
struct AttributeEntry;
class LocalParticipant;
class Participant;
class RemoteParticipant;
struct RoomInfoData;
class Track;
class TrackPublication;
}

struct MeetingSessionJoinOptions {
    bool autoConnectAudio = true;
    bool startCamera = true;
    bool startMicrophone = true;
};

struct MeetingSessionCtx {
    QString livekitUrl;
    QString livekitToken;
    QString meetingNumber;
    QString displayName;
    MeetingSessionJoinOptions joinOptions;

    bool isValid() const;
    static MeetingSessionCtx developmentDefaults();
};

enum class MeetingSessionRoomState {
    Disconnected,
    Connecting,
    Connected,
    Reconnecting,
    Disconnecting
};

enum class MeetingSessionMediaState {
    Off,
    Starting,
    On,
    Stopping,
    Failed
};

struct MeetingSessionRemoteParticipantInfo {
    QString participantId;
    QString name;
    QString metadata;
};

class MeetingSession : public QObject, public livekit::RoomDelegate
{
    Q_OBJECT

public:
    explicit MeetingSession(const MeetingSessionCtx &context, QObject *parent = nullptr);
    ~MeetingSession() override;

    const MeetingSessionCtx &context() const { return context_; }

    bool start();
    void shutdown();

    bool startAudio();
    void stopAudio();
    bool startVideo();
    void stopVideo();

    QString localParticipantId() const;
    QString localParticipantName() const;
    QString localVideoTrackSid() const;

    MeetingSessionRoomState roomState() const { return roomState_; }
    MeetingSessionMediaState microphoneState() const { return microphoneState_; }
    MeetingSessionMediaState cameraState() const { return cameraState_; }

    AudioLevelInfo localAudioLevel() const;
    bool isLocalAudioSpeaking() const;
    QHash<QString, AudioLevelInfo> remoteAudioLevels() const;
    QVector<MeetingSessionRemoteParticipantInfo> remoteUsers() const;

    // Participant display name resolution
    QString getParticipantDisplayName(const QString &participantId, const QString &name);

    // Track to participant mapping
    QString getParticipantIdByTrackSid(const QString &trackSid) const;
    void mapTrackToParticipant(const QString &trackSid, const QString &participantId);
    void unmapTrack(const QString &trackSid);

    // Cleanup participant data when participant leaves
    void clearParticipantData(const QString &participantId);

signals:
    void sigParticipantJoined(const QString &participantId, const QString &name);
    void sigParticipantLeft(const QString &participantId, const QString &name);
    void sigTrackSubscribed(const QString &trackSid, const QString &trackName,
        const QString &participantId, int trackKind);
    void sigTrackUnsubscribed(const QString &trackSid, const QString &trackName,
        const QString &participantId, int trackKind);
    void sigRoomStateChanged(MeetingSessionRoomState state);
    void sigMicrophoneStateChanged(MeetingSessionMediaState state);
    void sigCameraStateChanged(MeetingSessionMediaState state);
    void sigSessionError(const QString &message);

private:
    // Event data helpers
    struct ParticipantEventInfo {
        QString participantId;
        QString name;
    };

    struct TrackEventInfo {
        QString participantId;
        QString trackSid;
        QString trackName;
    };

    // Session lifecycle and media helpers
    void setRoomOptions(bool autoSubscribe = true, bool dynacast = false,
        bool e2ee = false, bool singlePeerConnection = false);
    bool connectRoom();
    bool disconnectRoom();
    livekit::LocalParticipant *getLocalParticipant() const;
    void resetSessionMediaState();
    void stopLocalMediaCapture(bool unpublishTracks);
    bool shouldStartMicrophoneOnJoin() const;
    bool shouldStartCameraOnJoin() const;
    void clearSessionCaches();

    // Event conversion and logging helpers
    ParticipantEventInfo buildParticipantEventInfo(livekit::Participant *participant) const;
    TrackEventInfo buildTrackEventInfo(livekit::Participant *participant,
        livekit::TrackPublication *publication) const;
    MeetingSessionRemoteParticipantInfo buildRemoteParticipantInfo(
        const livekit::RemoteParticipant *participant) const;
    QStringList buildChangedAttributes(const QVector<livekit::AttributeEntry> &attributes) const;
    void logRoomSnapshot(const char *eventName, const livekit::RoomInfoData &info) const;
    void startRemoteTrackMedia(const std::shared_ptr<livekit::Track> &track,
        const QString &trackSid) const;
    void stopRemoteTrackMedia(const std::shared_ptr<livekit::Track> &track,
        const QString &trackSid) const;

    // LiveKit room delegate callbacks
    void onParticipantConnected(livekit::Room &room, const livekit::ParticipantConnectedEvent &ev) override;
    void onParticipantsUpdated(livekit::Room &room, const livekit::ParticipantsUpdatedEvent &ev) override;
    void onParticipantDisconnected(livekit::Room &room, const livekit::ParticipantDisconnectedEvent &ev) override;
    void onLocalTrackPublished(livekit::Room &room, const livekit::LocalTrackPublishedEvent &ev) override;
    void onLocalTrackUnpublished(livekit::Room &room, const livekit::LocalTrackUnpublishedEvent &ev) override;
    void onLocalTrackSubscribed(livekit::Room &room, const livekit::LocalTrackSubscribedEvent &ev) override;
    void onTrackPublished(livekit::Room &room, const livekit::TrackPublishedEvent &ev) override;
    void onTrackUnpublished(livekit::Room &room, const livekit::TrackUnpublishedEvent &ev) override;
    void onTrackSubscribed(livekit::Room &room, const livekit::TrackSubscribedEvent &ev) override;
    void onTrackUnsubscribed(livekit::Room &room, const livekit::TrackUnsubscribedEvent &ev) override;
    void onTrackSubscriptionFailed(livekit::Room &room, const livekit::TrackSubscriptionFailedEvent &ev) override;
    void onTrackMuted(livekit::Room &room, const livekit::TrackMutedEvent &ev) override;
    void onTrackUnmuted(livekit::Room &room, const livekit::TrackUnmutedEvent &ev) override;
    void onActiveSpeakersChanged(livekit::Room &room, const livekit::ActiveSpeakersChangedEvent &ev) override;
    void onRoomMetadataChanged(livekit::Room &room, const livekit::RoomMetadataChangedEvent &ev) override;
    void onRoomSidChanged(livekit::Room &room, const livekit::RoomSidChangedEvent &ev) override;
    void onRoomUpdated(livekit::Room &room, const livekit::RoomUpdatedEvent &ev) override;
    void onRoomMoved(livekit::Room &room, const livekit::RoomMovedEvent &ev) override;
    void onParticipantMetadataChanged(livekit::Room &room, const livekit::ParticipantMetadataChangedEvent &ev) override;
    void onParticipantAttributesChanged(livekit::Room &room, const livekit::ParticipantAttributesChangedEvent &ev) override;
    void onParticipantEncryptionStatusChanged(livekit::Room &room, const livekit::ParticipantEncryptionStatusChangedEvent &ev) override;
    void onConnectionQualityChanged(livekit::Room &room, const livekit::ConnectionQualityChangedEvent &ev) override;
    void onConnectionStateChanged(livekit::Room &room, const livekit::ConnectionStateChangedEvent &ev) override;
    void onDisconnected(livekit::Room &room, const livekit::DisconnectedEvent &ev) override;
    void onReconnecting(livekit::Room &room, const livekit::ReconnectingEvent &ev) override;
    void onReconnected(livekit::Room &room, const livekit::ReconnectedEvent &ev) override;

    // State transition helpers
    void setRoomState(MeetingSessionRoomState state);
    void setMicrophoneState(MeetingSessionMediaState state);
    void setCameraState(MeetingSessionMediaState state);

    // Session context and room runtime
    MeetingSessionCtx context_;
    livekit::Room room_;

    // Local participant/session media cache
    livekit::LocalParticipant *localParticipant_ = nullptr;
    QString localAudioTrackSid_;
    QString localVideoTrackSid_;
    QString e2eeKey_ = QStringLiteral(LIVEKIT_E2EE_KEY);
    livekit::RoomOptions roomOptions_ = {0};

    // Session state machine
    MeetingSessionRoomState roomState_ = MeetingSessionRoomState::Disconnected;
    MeetingSessionMediaState microphoneState_ = MeetingSessionMediaState::Off;
    MeetingSessionMediaState cameraState_ = MeetingSessionMediaState::Off;

    // Participant and track caches
    QHash<QString, QString> participantDisplayNames_;
    int nextGuestIndex_ = 1;
    QHash<QString, QString> trackToParticipantMap_;
};

#endif // MEETING_SESSION_H