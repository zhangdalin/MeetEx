#ifndef LOCAL_USER_H
#define LOCAL_USER_H

#include "livekit/livekit.h"

struct VideoFrameBuff;

class LocalUser {
public:
    explicit LocalUser(livekit::LocalParticipant* participant);
    ~LocalUser();

    std::string identity() const;
    std::string name() const;
    std::string metadata() const;
    const std::string& videoSid() const { return video_sid_; }
    const std::string& audioSid() const { return audio_sid_; }

    bool openAudio();
    void closeAudio();

    bool openVideo(std::string& localVideoSid);
    void closeVideo();

    void closeSpeaker();
    void closeRenderer();

private:
    livekit::LocalParticipant* participant_;
    std::string audio_sid_;
    std::string video_sid_;
};

#endif // LOCAL_USER_H