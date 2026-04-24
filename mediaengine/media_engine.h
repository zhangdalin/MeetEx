#ifndef MEDIA_ENGINE_H
#define MEDIA_ENGINE_H

#include "media_mgr.h"

#include "livekit/local_participant.h"

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

    bool startAudioPlay(const std::shared_ptr<livekit::AudioStream> &audio_stream, const std::string& track_sid);
    bool startVideoRender(const std::shared_ptr<livekit::VideoStream> &video_stream, const std::string &track_sid);

    void stopAudioPlay(const std::string& track_sid);
    void stopVideoRender(const std::string& track_sid);

    void stopAllAudioPlay();
    void stopAllVideoRender();

    bool copyVideoFrame(const std::string &track_sid, VideoFrameBuff& frameBuff);
    AudioLevelInfo localAudioLevel() const;
    bool isLocalAudioSpeaking() const;
    AudioLevelInfo remoteAudioLevel(const std::string &track_sid) const;
    bool isRemoteAudioSpeaking(const std::string &track_sid) const;
    std::unordered_map<std::string, AudioLevelInfo> remoteAudioLevels() const;

private:
    explicit MediaEngine();
    ~MediaEngine() = default;

    std::shared_ptr<MediaMgr> media_mgr_;
};

#endif // MEDIA_ENGINE_H