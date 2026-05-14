#ifndef MEETING_SESSION_H
#define MEETING_SESSION_H

#include "meeting_def.h"
#include "meeting_participant.h"
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
#include <unordered_map>

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
    bool startCamera = true;
    bool startMicrophone = true;
    bool startScreenShare = false;
    bool startRecording = false;
};

struct MeetingSessionCtx {
    QString livekitUrl;
    QString livekitToken;
    QString meetingNumber;
    QString displayName;
    MeetingSessionJoinOptions joinOptions;
    bool isValid() const;
    static MeetingSessionCtx defaults();
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

class MeetingSession : public QObject, public livekit::RoomDelegate
{
    Q_OBJECT

public:
    explicit MeetingSession(const MeetingSessionCtx &context, QObject *parent = nullptr);
    ~MeetingSession();

    const MeetingSessionCtx &context() const { return context_; }

    bool start();
    void shutdown();

    const MeetingParticipant &localParticipant() const { return localParticipant_; }
    const QHash<QString, MeetingParticipant> &remoteParticipants() const { return remoteParticipants_; }

    bool startAudio();
    void stopAudio();
    bool startVideo();
    void stopVideo();
    bool startShare();
    void stopShare();
    bool startRecording();
    void stopRecording();

    MeetingSessionRoomState roomState() const { return roomState_; }
    MeetingSessionMediaState microphoneState() const { return microphoneState_; }
    MeetingSessionMediaState cameraState() const { return cameraState_; }
    MeetingSessionMediaState screenShareState() const { return screenShareState_; }
    MeetingSessionMediaState recordingState() const { return recordingState_; }

    AudioLevelInfo localAudioLevel() const;
    bool isLocalAudioSpeaking() const;
    std::unordered_map<std::string, AudioLevelInfo> remoteAudioLevels() const;
    const MeetingParticipant* findParticipantByTrackSid(const QString &trackSid, int trackKind) const;

signals:
    void sigParticipantJoined(const QString &participantId);
    void sigParticipantLeft(const QString &participantId);
    void sigTrackSubscribed(const QString &participantId, int trackKind);
    void sigTrackUnsubscribed(const QString &participantId, int trackKind);

    void sigRoomStateChanged(MeetingSessionRoomState state);
    void sigMicrophoneStateChanged(MeetingSessionMediaState state);
    void sigCameraStateChanged(MeetingSessionMediaState state);
    void sigScreenShareStateChanged(MeetingSessionMediaState state);
    void sigRecordingStateChanged(MeetingSessionMediaState state);
    void sigSessionError(const QString &message);

private:
    // Session lifecycle and media helpers
    void setRoomOptions(bool autoSubscribe = true, bool dynacast = false,
        bool e2ee = false, bool singlePeerConnection = false);

    bool connectRoom();
    void disconnectRoom();
    void syncLocalParticipant();
    void syncRemoteParticipants();

    // State transition helpers
    void setRoomState(MeetingSessionRoomState state);
    void setMicrophoneState(MeetingSessionMediaState state);
    void setCameraState(MeetingSessionMediaState state);
    void setScreenShareState(MeetingSessionMediaState state);
    void setRecordingState(MeetingSessionMediaState state);

    bool shouldStartMicrophoneOnJoin() const;
    bool shouldStartCameraOnJoin() const;
    bool shouldStartScreenShareOnJoin() const;
    bool shouldStartRecordingOnJoin() const;

    QStringList buildChangedAttributes(const QVector<livekit::AttributeEntry> &attributes) const;
    void logRoomSnapshot(const char *eventName, const livekit::RoomInfoData &info) const;
    void startRemoteTrackMedia(const std::shared_ptr<livekit::Track> &track) const;
    void stopRemoteTrackMedia(const std::shared_ptr<livekit::Track> &track) const;

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

private:
    // Session context and room runtime
    MeetingSessionCtx context_;
    livekit::Room room_;

    QString e2eeKey_ = QStringLiteral(LIVEKIT_E2EE_KEY);
    livekit::RoomOptions roomOptions_ = {0};

    // Session state machine
    MeetingSessionRoomState roomState_ = MeetingSessionRoomState::Disconnected;
    MeetingSessionMediaState microphoneState_ = MeetingSessionMediaState::Off;
    MeetingSessionMediaState cameraState_ = MeetingSessionMediaState::Off;
    MeetingSessionMediaState screenShareState_ = MeetingSessionMediaState::Off;
    MeetingSessionMediaState recordingState_ = MeetingSessionMediaState::Off;

    MeetingParticipant localParticipant_;
    QHash<QString, MeetingParticipant> remoteParticipants_;
    std::string screenShareTrackSid_;
};

#endif // MEETING_SESSION_H