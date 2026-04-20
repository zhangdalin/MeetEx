#ifndef MEDIA_MGR_H
#define MEDIA_MGR_H

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace livekit
{
class AudioSource;
class VideoSource;
class AudioStream;
class VideoStream;
} // namespace livekit

class QMicSource;
class QCamSource;

struct VideoFrameBuff {
    std::mutex mutex;
    std::vector<uint8_t> rgba;
    int width = 0;
    int height = 0;
};

// MediaMgr gives you dedicated control over:
// - mic capture  -> AudioSource
// - camera capture -> VideoSource
// - speaker playback -> AudioStream (TODO: integrate your API)
// - renderer -> VideoStream (TODO: integrate your API)
class MediaMgr
{
public:
    MediaMgr();
    ~MediaMgr();

    // Mic (local capture -> AudioSource)
    bool startMic(const std::shared_ptr<livekit::AudioSource> &audio_source);
    void stopMic();

    // Camera (local capture -> VideoSource)
    bool startCamera(const std::shared_ptr<livekit::VideoSource> &video_source, const std::string &track_sid);
    void stopCamera();

    // Playback (remote audio playback)
    bool startPlayback(const std::shared_ptr<livekit::AudioStream> &audio_stream, const std::string& track_sid);
    void stopAllPlayback();

    // Renderer (remote video rendering)
    // Following APIs must be called on main thread
    bool startRender(const std::shared_ptr<livekit::VideoStream> &video_stream, const std::string &track_sid);
    void stopAllRenders();

    bool copyVideoFrame(const std::string &track_sid, VideoFrameBuff& frameBuff);

private:
    struct PlaybackWorker {
        std::shared_ptr<livekit::AudioStream> stream;
        std::thread thread;
        std::atomic<bool> running{false};
    };

    struct RenderWorker {
        std::shared_ptr<livekit::VideoStream> stream;
        std::thread thread;
        std::atomic<bool> running{false};
        VideoFrameBuff frameBuff;
    };

    // ---- Audio mixer ----
    // All remote audio tracks are mixed into a single QSpkSink output.
    struct MixTrack {
        std::vector<int16_t> buf; // pending interleaved S16 samples
    };
    
    void micLoop(const std::shared_ptr<livekit::AudioSource> &source,
                        std::atomic<bool> &running_flag);
    void runNoiseCapLoop(const std::shared_ptr<livekit::AudioSource> &source,
                        std::atomic<bool> &running_flag);
    void runFakeVideoCapLoop(const std::shared_ptr<livekit::VideoSource> &source,
                        VideoFrameBuff& frameBuff,
                        std::atomic<bool> &running_flag);

    // ---- Playback helpers ----
    void playbackLoop(const std::string &track_sid, const std::shared_ptr<PlaybackWorker> &worker);
    void renderLoop(const std::string &track_sid, const std::shared_ptr<RenderWorker> &worker);

    void mixLoop();

    std::mutex              mix_mutex_;
    std::condition_variable mix_cv_;
    std::unordered_map<std::string, MixTrack> mix_tracks_;
    std::thread             mix_thread_;
    std::atomic<bool>       mix_running_{false};

    // Mic
    std::shared_ptr<livekit::AudioSource> mic_source_;
    std::thread mic_thread_;
    std::atomic<bool> mic_running_{false};
    bool mic_using_ = false;

    // Camera
    std::shared_ptr<livekit::VideoSource> cam_source_;
    std::unique_ptr<QCamSource> cam_;
    std::thread cam_thread_;
    std::atomic<bool> cam_running_{false};
    bool cam_using_ = false;

    // Playback (remote audio)
    std::mutex playback_mutex_;
    std::unordered_map<std::string, std::shared_ptr<PlaybackWorker>> playback_;

    // Renderer (remote video)
    std::mutex renders_mutex_;
    std::unordered_map<std::string, std::shared_ptr<RenderWorker>> renders_;
};

#endif // MEDIA_MGR_H