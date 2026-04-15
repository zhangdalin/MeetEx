#ifndef MEDIA_ENGINE_H
#define MEDIA_ENGINE_H

#include "media_mgr.h"

#include <cstdint>
#include <vector>

class MediaMgr;

class MediaEngine {
public:
    static MediaEngine &instance() {
        static MediaEngine instance;
        return instance;
    }

    static void printLiveKitVersion();

    bool init();
    bool fini();
    bool startLocalAudio(livekit::LocalParticipant* participant, std::string& sid);
    void stopLocalAudio(livekit::LocalParticipant* participant, const std::string& sid);
    bool startLocalVideo(livekit::LocalParticipant* participant, std::string& sid);
    void stopLocalVideo(livekit::LocalParticipant* participant, const std::string& sid);
    bool startShareLocalScreen(livekit::LocalParticipant* participant, std::string& sid);
    void stopShareLocalScreen(livekit::LocalParticipant* participant, const std::string& sid);
    bool startAudioSpeaker(const std::shared_ptr<livekit::AudioStream> &audio_stream);
    bool startVideoRender(const std::shared_ptr<livekit::VideoStream> &video_stream, const std::string &track_sid);
    void stopAudioSpeaker();
    void stopVideoRender();
    bool copyLatestVideoFrame(const std::string &track_sid, std::vector<std::uint8_t> &rgba, int &width, int &height);

private:
    explicit MediaEngine();
    ~MediaEngine() = default;

    std::shared_ptr<MediaMgr> media_mgr_;
};

#endif // MEDIA_ENGINE_H