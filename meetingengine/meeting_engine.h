#ifndef MEETING_ENGINE_H
#define MEETING_ENGINE_H

#include <memory>

#include <QObject>
#include <QString>

class MeetingRoom;
class MeetingEngine : public QObject {
    Q_OBJECT

public:
    explicit MeetingEngine(QObject *parent = nullptr);
    ~MeetingEngine();

    bool launchMeeting();
    bool joinMeeting();
    void endMeeting();

    bool startAudio();
    void stopAudio();
    bool startVideo();
    void stopVideo();

signals:
    void sigParticipantJoined(const QString &participantId, const QString &participantName);
    void sigTrackSubscribed(const QString &trackSid, const QString &trackName, const QString &participantIdentity, int trackKind);

private:
    std::unique_ptr<MeetingRoom> room_;
};

#endif // MEETING_ENGINE_H