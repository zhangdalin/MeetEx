#ifndef MEETING_ENGINE_H
#define MEETING_ENGINE_H

#include <memory>

class MeetingRoom;
class MeetingEngine {
public:
    explicit MeetingEngine();
    ~MeetingEngine();

    bool launchMeeting();
    bool joinMeeting();
    void endMeeting();

    MeetingRoom* room() const { return room_.get(); }

    bool startAudio();
    void stopAudio();
    bool startVideo();
    void stopVideo();

private:
    std::unique_ptr<MeetingRoom> room_;
};

#endif // MEETING_ENGINE_H