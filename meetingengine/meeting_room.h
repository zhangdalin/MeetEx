#ifndef MEETING_ROOM_H
#define MEETING_ROOM_H

#include <string>
#include <memory>

#include <QString>
#include <QStringList>
#include <QObject>

#include "livekit/room.h"
#include "livekit/room_delegate.h"

enum class RoomState {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    DISCONNECTING
};

class LocalUser;
class RemoteUser;

class MeetingRoom : public QObject, public livekit::Room, public livekit::RoomDelegate {
    Q_OBJECT

public:
    explicit MeetingRoom();
    ~MeetingRoom();

    void setConnectionInfo(const std::string &url, const std::string &token,
        const std::string &e2eeKey = std::string());

    void setRoomOptions(bool auto_subscribe = true, bool dynacast = false, 
        bool e2ee = false, bool single_peer_connection = false);

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


    bool connect();
    bool disconnect();

    std::shared_ptr<LocalUser>& getLocalUser();
    std::shared_ptr<RemoteUser> getRemoteUser(const std::string &id);
    std::vector<std::shared_ptr<RemoteUser>> getRemoteUsers();

signals:
    void sigParticipantJoined(const QString &participantId, const QString &participantName);
    void sigParticipantLeft(const QString &participantId, const QString &participantName);

    void sigLocalTrackPublished(const QString &trackSid, const QString &trackName);
    void sigLocalTrackUnpublished(const QString &trackSid, const QString &trackName);
    void sigLocalTrackSubscribed(const QString &trackSid, const QString &trackName);

    void sigTrackPublished(const QString &trackSid, const QString &trackName, const QString &participantId);
    void sigTrackUnpublished(const QString &trackSid, const QString &trackName, const QString &participantId);
    void sigTrackSubscribed(const QString &trackSid, const QString &trackName, const QString &participantId, int trackKind);
    void sigTrackUnsubscribed(const QString &trackSid, const QString &trackName, const QString &participantId, int trackKind);
    void sigTrackSubscriptionFailed(const QString &trackSid, const QString &participantId, const QString &errorMessage);
    void sigTrackMuted(const QString &trackSid, const QString &trackName, const QString &participantId);
    void sigTrackUnmuted(const QString &trackSid, const QString &trackName, const QString &participantId);

    void sigActiveSpeakersChanged(const QStringList &participantIds);
    void sigRoomMetadataChanged(const QString &oldMetadata, const QString &newMetadata);
    void sigRoomSidChanged(const QString &newSid);
    void sigRoomUpdated();
    void sigRoomMoved();

    void sigParticipantMetadataChanged(const QString &participantId, const QString &oldMetadata, const QString &newMetadata);
    void sigParticipantAttributesChanged(const QString &participantId, const QStringList &changedAttributes);
    void sigParticipantEncryptionStatusChanged(const QString &participantId, bool encrypted);

    void sigConnectionQualityChanged(const QString &participantId, livekit::ConnectionQuality quality);
    void sigConnectionStateChanged(livekit::ConnectionState state);
    void sigDisconnected(livekit::DisconnectReason reason);
    void sigReconnecting();
    void sigReconnected();

private:
    std::string url_;
    std::string token_;
    std::string e2ee_key_;
    livekit::RoomOptions options_;
    std::shared_ptr<LocalUser> localUser_;
    RoomState state_;
};

#endif // MEETING_ROOM_H