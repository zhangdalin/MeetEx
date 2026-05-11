#ifndef MEETING_PARTICIPANT_H
#define MEETING_PARTICIPANT_H

#include <QString>

namespace livekit {
class Participant;
}

class MeetingParticipant {
public:
    MeetingParticipant() = default;
    explicit MeetingParticipant(const livekit::Participant *participant, bool isLocal = false);

    bool isValid() const;
    const QString& id() const;
    const QString& name() const;
    const QString& metadata() const;
    const QString& audioTrackSid() const;
    const QString& videoTrackSid() const;
    bool isLocal() const;

    void setId(const QString &id);
    void setName(const QString &name);
    void setMetadata(const QString &metadata);
    void setAudioTrackSid(const QString &trackSid);
    void setVideoTrackSid(const QString &trackSid);
    void setIsLocal(bool isLocal);
    void syncFromLivekit(const livekit::Participant *participant);

    void clear();

private:
    QString participantId_;
    QString participantName_;
    QString participantMetadata_;
    QString audioTrackSid_;
    QString videoTrackSid_;
    bool isLocal_ = false;
};

#endif // MEETING_PARTICIPANT_H
