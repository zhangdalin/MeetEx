#ifndef MEETING_ROOM_H
#define MEETING_ROOM_H

#include <string>
#include <memory>
#include <functional>

#include <QString>
#include <QObject>

#include "livekit/livekit.h"

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

    void setRoomOptions(bool auto_subscribe = true, bool dynacast = false, 
        bool e2ee = false, bool single_peer_connection = false);

    void onParticipantConnected(livekit::Room &room, const livekit::ParticipantConnectedEvent &ev) override;
    void onTrackSubscribed(livekit::Room &room, const livekit::TrackSubscribedEvent &ev) override;

    bool connect();
    bool disconnect();

    std::shared_ptr<LocalUser>& getLocalUser();
    std::shared_ptr<RemoteUser> getRemoteUser(const std::string &identity);

signals:
    void sigParticipantJoined(const QString &participantId, const QString &participantName);
    void sigTrackSubscribed(const QString &trackSid, const QString &trackName, const QString &participantIdentity, int trackKind);

private:
    std::string url_;
    std::string token_;
    std::string e2ee_key_;
    livekit::RoomOptions options_;
    std::shared_ptr<LocalUser> localUser_;
    RoomState state_;
};

#endif // MEETING_ROOM_H