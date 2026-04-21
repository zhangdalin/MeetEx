#ifndef MEETING_ENGINE_H
#define MEETING_ENGINE_H

#include "media_mgr.h"

#include <memory>
#include <string>

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

    bool startVideo(std::string& localVideoSid);
    void stopVideo();
    std::string localUserIdentity() const;
    std::string localVideoSid() const;
    AudioLevelInfo localAudioLevel() const;
    bool isLocalAudioSpeaking() const;
    AudioLevelInfo remoteAudioLevel(const std::string &trackSid) const;
    bool isRemoteAudioSpeaking(const std::string &trackSid) const;
    std::unordered_map<std::string, AudioLevelInfo> remoteAudioLevels() const;

private:
    std::unique_ptr<MeetingRoom> room_;
};

#endif // MEETING_ENGINE_H