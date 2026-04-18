#ifndef MEETING_ENGINE_H
#define MEETING_ENGINE_H

#include <memory>
#include <string>
#include <vector>

class MeetingRoom;
struct VideoFrameBuff;
class MeetingEngine {
public:
    explicit MeetingEngine();
    ~MeetingEngine();

    bool joinMeeting();
    void endMeeting();

    MeetingRoom* room() const { return room_.get(); }

    bool startAudio();
    void stopAudio();

    bool startVideo(std::string& localVideoSid, VideoFrameBuff* frameBuff);
    void stopVideo();
    std::string localUserIdentity() const;
    std::string localVideoSid() const;

private:
    std::unique_ptr<MeetingRoom> room_;
};

#endif // MEETING_ENGINE_H