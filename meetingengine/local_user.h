#ifndef LOCAL_USER_H
#define LOCAL_USER_H

#include "livekit/livekit.h"

class LocalUser {
public:
    explicit LocalUser(livekit::LocalParticipant* participant);
    ~LocalUser();

    std::string identity() const;
    std::string name() const;
    std::string metadata() const;

    bool openAudio();
    void closeAudio();
    bool openVideo();
    void closeVideo();
    void closeSpeaker();
    void closeRenderer();

private:
    livekit::LocalParticipant* participant_;
    std::string audio_sid_;
    std::string video_sid_;
};

#endif // LOCAL_USER_H