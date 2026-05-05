#ifndef MEETING_SESSION_H
#define MEETING_SESSION_H

#include "meeting_def.h"
#include "media_mgr.h"

#include <QObject>
#include <QString>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class MeetingEngine;
class RemoteUser;

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

class MeetingSession : public QObject
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
    QString localVideoTrackSid() const;

    MeetingSessionRoomState roomState() const { return roomState_; }
    MeetingSessionMediaState microphoneState() const { return microphoneState_; }
    MeetingSessionMediaState cameraState() const { return cameraState_; }

    AudioLevelInfo localAudioLevel() const;
    bool isLocalAudioSpeaking() const;
    std::unordered_map<std::string, AudioLevelInfo> remoteAudioLevels() const;
    std::vector<std::shared_ptr<RemoteUser>> remoteUsers() const;

    // Participant display name resolution
    QString getParticipantDisplayName(const QString &participantId, const QString &participantName);

    // Track to participant mapping
    QString getParticipantIdByTrackSid(const QString &trackSid) const;
    void mapTrackToParticipant(const QString &trackSid, const QString &participantId);
    void unmapTrack(const QString &trackSid);

    // Cleanup participant data when participant leaves
    void clearParticipantData(const QString &participantId);

signals:
    void sigParticipantJoined(const QString &participantId, const QString &participantName);
    void sigParticipantLeft(const QString &participantId, const QString &participantName);
    void sigTrackSubscribed(const QString &trackSid, const QString &trackName,
        const QString &participantId, int trackKind);
    void sigTrackUnsubscribed(const QString &trackSid, const QString &trackName,
        const QString &participantId, int trackKind);
    void sigRoomStateChanged(MeetingSessionRoomState state);
    void sigMicrophoneStateChanged(MeetingSessionMediaState state);
    void sigCameraStateChanged(MeetingSessionMediaState state);
    void sigSessionError(const QString &message);

private:
    void bindEngineSignals();
    void setRoomState(MeetingSessionRoomState state);
    void setMicrophoneState(MeetingSessionMediaState state);
    void setCameraState(MeetingSessionMediaState state);

    MeetingSessionCtx context_;
    std::unique_ptr<MeetingEngine> engine_;
    MeetingSessionRoomState roomState_ = MeetingSessionRoomState::Disconnected;
    MeetingSessionMediaState microphoneState_ = MeetingSessionMediaState::Off;
    MeetingSessionMediaState cameraState_ = MeetingSessionMediaState::Off;
    std::string localVideoTrackSid_;
    bool started_ = false;

    // Participant name cache: participantId -> generated name if empty
    std::unordered_map<std::string, std::string> participantDisplayNames_;
    int nextGuestIndex_ = 1;

    // Track SID to participant ID mapping
    std::unordered_map<std::string, std::string> trackToParticipantMap_;
};

#endif // MEETING_SESSION_H