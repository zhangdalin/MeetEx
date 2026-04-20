#include "media_mgr.h"
#include "media_qmic.h"
#include "media_qcam.h"
#include "media_qspk.h"
#include "fallback_capture.h"

#include <QCameraDevice>
#include <QAudioDevice>
#include <QMediaDevices>

#include <QDebug>

#include <livekit/livekit.h>

MediaMgr::MediaMgr() = default;

MediaMgr::~MediaMgr() {
    stopMic();
    stopCamera();
    stopAllPlayback();
    stopAllRenders();
}

// ---------- Mic control ----------

bool MediaMgr::startMic(const std::shared_ptr<livekit::AudioSource> &audio_source) {
    stopMic();

    if (!audio_source) {
        qCritical() << __FUNCTION__ << "audioSource is null";
        return false;
    }

    mic_source_ = audio_source;
    mic_running_.store(true, std::memory_order_relaxed);

    if (QMediaDevices::audioInputs().isEmpty()) {
        qWarning() << __FUNCTION__ << "No microphone devices found, falling back to noise loop.";
        mic_using_ = false;
        mic_thread_ = std::thread(runNoiseCaptureLoop, mic_source_, std::ref(mic_running_));
        return true;
    }

    mic_using_ = true;
    mic_thread_ = std::thread(&MediaMgr::micLoop, this);
    return true;
}

void MediaMgr::micLoop() {
    auto source = mic_source_;
    QMicSource mic(source->sample_rate(),
                  source->num_channels(),
                  source->sample_rate() / 100,
                  [source](const int16_t *samples,
                           int num_samples_per_channel,
                           int sample_rate, int num_channels) {
                      livekit::AudioFrame frame = livekit::AudioFrame::create(sample_rate, num_channels,
                                                                              num_samples_per_channel);
                      std::memcpy(frame.data().data(), samples,
                                  num_samples_per_channel * num_channels * sizeof(int16_t));
                      try {
                          source->captureFrame(frame);
                      } catch (const std::exception &e) {
                          qCritical() << __FUNCTION__ << "Error in captureFrame (Qt mic):" << e.what();
                      }
                  });

    if (!mic.init()) {
        qWarning() << __FUNCTION__ << "Failed to init Qt mic, falling back to noise loop.";
        runNoiseCaptureLoop(source, mic_running_);
        return;
    }

    while (mic_running_.load(std::memory_order_relaxed)) {
        mic.pump();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void MediaMgr::stopMic() {
    mic_running_.store(false, std::memory_order_relaxed);
    if (mic_thread_.joinable()) {
        mic_thread_.join();
    }
    mic_source_.reset();
}

// ---------- Camera control ----------

bool MediaMgr::startCamera(const std::shared_ptr<livekit::VideoSource> &video_source, const std::string &track_sid) {
    stopCamera();

    if (!video_source) {
        qCritical() << __FUNCTION__ << "videoSource is null";
        return false;
    }

    if (track_sid.empty()) {
        qCritical() << __FUNCTION__ << "track_sid is empty";
        return false;
    }

    auto worker = std::make_shared<RenderWorker>();
    worker->stream.reset();
    worker->running.store(true, std::memory_order_relaxed);

    std::shared_ptr<RenderWorker> old_worker = nullptr;
    {
        std::lock_guard<std::mutex> lock(renders_mutex_);
        auto it = renders_.find(track_sid);
        if (it != renders_.end()) {
            old_worker = it->second;
        }
        renders_[track_sid] = worker;
    }

    if (old_worker) {
        old_worker->running.store(false, std::memory_order_relaxed);
    }

    cam_source_ = video_source;
    cam_running_.store(true, std::memory_order_relaxed);

    // Check for available cameras via Qt
    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    if (cameras.isEmpty()) {
        qWarning() << __FUNCTION__ << "No camera devices found, using fake video loop.";
        cam_using_ = false;
        cam_thread_ = std::thread(runFakeVideoCaptureLoop, cam_source_, 
            std::ref(worker->frameBuff), std::ref(cam_running_));
        return true;
    }

    cam_using_ = true;
    cam_ = std::make_unique<QCamSource>(
        1280, 720, 30,
        [src = cam_source_, buff = &(worker->frameBuff)](const uint8_t *pixels, int pitch, int width,
                            int height, int64_t timestampNs) {
            auto frame = livekit::VideoFrame::create(width, height, livekit::VideoBufferType::RGBA);
            uint8_t *dst = frame.data();
            const int dstPitch = width * 4;

            for (int y = 0; y < height; ++y) {
                std::memcpy(dst + y * dstPitch, pixels + y * pitch, dstPitch);
            }

            // add frame to local render
            if (buff) {
                std::lock_guard<std::mutex> lock(buff->mutex);
                const int rgba_size = width * height * 4;
                std::vector<std::uint8_t> rgba(static_cast<size_t>(rgba_size));
                std::memcpy(rgba.data(), frame.data(), static_cast<size_t>(rgba_size));
                buff->rgba = std::move(rgba);
                buff->width = width;
                buff->height = height;
            }

            // add frame to video source;
            try {
                src->captureFrame(frame, timestampNs / 1000, livekit::VideoRotation::VIDEO_ROTATION_0);
            } catch (const std::exception &e) {
                qCritical() << __FUNCTION__ << "Error in captureFrame (Qt cam):" << e.what();
            }
        });

    if (!cam_->init()) {
        qWarning() << __FUNCTION__ << "Failed to init Qt camera, using fake video loop.";
        cam_using_ = false;
        cam_.reset();
        cam_thread_ = std::thread(runFakeVideoCaptureLoop, cam_source_, 
            std::ref(worker->frameBuff), std::ref(cam_running_));
        return true;
    }

    // QCamSource delivers frames via Qt's event system; no pump thread needed.
    return true;
}

void MediaMgr::stopCamera() {
    cam_running_.store(false, std::memory_order_relaxed);
    if (cam_thread_.joinable()) {
        cam_thread_.join();
    }
    if (cam_) {
        cam_->stop();
    }
    cam_.reset();
    cam_source_.reset();
}

// ---------- Speaker control ----------

bool MediaMgr::startPlayback(const std::shared_ptr<livekit::AudioStream> &audio_stream, const std::string& track_sid) {
    if (!audio_stream) {
        qCritical() << __FUNCTION__ << "audioStream is null";
        return false;
    }

    if (track_sid.empty()) {
        qCritical() << __FUNCTION__ << "track_sid is empty";
        return false;
    }

    auto worker = std::make_shared<PlaybackWorker>();
    worker->stream = audio_stream;
    worker->running.store(true, std::memory_order_relaxed);

    std::shared_ptr<PlaybackWorker> old_worker = nullptr;
    {
        std::lock_guard<std::mutex> lock(playback_mutex_);
        auto it = playback_.find(track_sid);
        if (it != playback_.end()) {
            old_worker = it->second;
        }
        playback_[track_sid] = worker;
    }

    if (old_worker) {
        old_worker->running.store(false, std::memory_order_relaxed);
        if (old_worker->thread.joinable()) {
            old_worker->thread.join();
        }
        old_worker->stream.reset();
    }

    try {
        worker->thread = std::thread(&MediaMgr::playbackLoop, this, track_sid, worker);
    } catch (const std::exception &e) {
        qCritical() << __FUNCTION__
                    << "failed to start playback thread for track_sid:" << QString::fromStdString(track_sid)
                    << "error:" << e.what();
        worker->running.store(false, std::memory_order_relaxed);
        worker->stream.reset();
        {
            std::lock_guard<std::mutex> lock(playback_mutex_);
            auto it = playback_.find(track_sid);
            if (it != playback_.end() && it->second == worker) {
                playback_.erase(it);
            }
        }
        return false;
    }

    return true;
}

void MediaMgr::playbackLoop(const std::string &track_sid, const std::shared_ptr<PlaybackWorker> &worker) {
    std::unique_ptr<QSpkSink> sink = nullptr;

    while (worker->running.load(std::memory_order_relaxed)) {
        if (!worker->stream) {
            break;
        }

        livekit::AudioFrameEvent ev;
        if (!worker->stream->read(ev)) {
            // EOS or closed
            break;
        }

        const livekit::AudioFrame &frame = ev.frame;
        const auto &data = frame.data();
        if (data.empty()) {
            continue;
        }

        if (!sink) {
            sink = std::make_unique<QSpkSink>(frame.sample_rate(), frame.num_channels());
            if (!sink->init()) {
                qCritical() << __FUNCTION__ << "Failed to init Qt speaker sink";
                break;
            }
        }

        sink->enqueue(data.data(), frame.samples_per_channel());
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    worker->running.store(false, std::memory_order_relaxed);

    {
        std::lock_guard<std::mutex> lock(playback_mutex_);
        auto it = playback_.find(track_sid);
        if (it != playback_.end() && it->second == worker) {
            playback_.erase(it);
        }
    }
}

void MediaMgr::stopAllPlayback() {
    std::vector<std::shared_ptr<PlaybackWorker>> workers;
    {
        std::lock_guard<std::mutex> lock(playback_mutex_);
        for (auto &entry : playback_) {
            workers.push_back(entry.second);
        }
        playback_.clear();
    }

    for (auto &worker : workers) {
        worker->running.store(false, std::memory_order_relaxed);
    }

    for (auto &worker : workers) {
        if (worker->thread.joinable()) {
            worker->thread.join();
        }
        worker->stream.reset();
    }
}

// ---------- Renderer control (placeholder) ----------

bool MediaMgr::startRender(const std::shared_ptr<livekit::VideoStream> &video_stream,
                             const std::string &track_sid) {
    if (!video_stream) {
        qCritical() << __FUNCTION__ << "videoStream is null";
        return false;
    }

    auto worker = std::make_shared<RenderWorker>();
    worker->stream = video_stream;
    worker->running.store(true, std::memory_order_relaxed);

    std::shared_ptr<RenderWorker> old_worker = nullptr;
    {
        std::lock_guard<std::mutex> lock(renders_mutex_);
        auto it = renders_.find(track_sid);
        if (it != renders_.end()) {
            old_worker = it->second;
        }
        renders_[track_sid] = worker;
    }

    if (old_worker) {
        old_worker->running.store(false, std::memory_order_relaxed);
        if (old_worker->thread.joinable()) {
            old_worker->thread.join();
        }
    }

    try {
        worker->thread = std::thread(&MediaMgr::renderLoop, this, track_sid, worker);
    } catch (const std::exception &e) {
        qCritical() << __FUNCTION__ 
                << "failed to start track_sid:" << QString::fromStdString(track_sid) 
                << "thread:" << e.what();

        {
            std::lock_guard<std::mutex> lock(renders_mutex_);
            auto it = renders_.find(track_sid);
            if (it != renders_.end() && it->second == worker) {
                renders_.erase(it);
            }
        }

        return false;
    }

    return true;
}

void MediaMgr::stopAllRenders() {
    std::vector<std::shared_ptr<RenderWorker>> workers;
    {
        std::lock_guard<std::mutex> lock(renders_mutex_);
        for (auto &entry : renders_) {
            workers.push_back(entry.second);
        }
        renders_.clear();
    }

    for (auto &worker : workers) {
        worker->running.store(false, std::memory_order_relaxed);
    }

    for (auto &worker : workers) {
        if (worker->thread.joinable()) {
            worker->thread.join();
        }
        worker->stream.reset();
    }
}

void MediaMgr::renderLoop(const std::string &track_sid,
                          const std::shared_ptr<RenderWorker> &worker) {
    while (worker->running.load(std::memory_order_relaxed)) {
        if (!worker->stream) {
            break;
        }

        livekit::VideoFrameEvent vfe;
        if (!worker->stream->read(vfe)) {
            break;
        }

        livekit::VideoFrame &frame = vfe.frame;
        if (frame.type() != livekit::VideoBufferType::RGBA) {
            try {
                frame = frame.convert(livekit::VideoBufferType::RGBA, false);
            } catch (const std::exception &ex) {
                qCritical() << __FUNCTION__ << "convert to RGBA failed:" << ex.what();
                continue;
            }
        }

        const int width = frame.width();
        const int height = frame.height();
        if (width <= 0 || height <= 0) {
            continue;
        }

        const int rgba_size = width * height * 4;
        std::vector<std::uint8_t> rgba(static_cast<size_t>(rgba_size));
        std::memcpy(rgba.data(), frame.data(), static_cast<size_t>(rgba_size));

        {
            std::lock_guard<std::mutex> lock(worker->frameBuff.mutex);
            worker->frameBuff.rgba = std::move(rgba);
            worker->frameBuff.width = width;
            worker->frameBuff.height = height;
        }
    }

    worker->running.store(false, std::memory_order_relaxed);
}

bool MediaMgr::copyVideoFrame(const std::string &track_sid, VideoFrameBuff& frameBuff) {
    std::shared_ptr<RenderWorker> worker;
    {
        std::lock_guard<std::mutex> lock(renders_mutex_);
        auto it = renders_.find(track_sid);
        if (it == renders_.end()) {
            return false;
        }
        worker = it->second;
    }

    std::lock_guard<std::mutex> lock(worker->frameBuff.mutex);
    if (worker->frameBuff.rgba.empty() || worker->frameBuff.width <= 0 || worker->frameBuff.height <= 0) {
        return false;
    }

    frameBuff.rgba = worker->frameBuff.rgba;
    frameBuff.width = worker->frameBuff.width;
    frameBuff.height = worker->frameBuff.height;
    return true;
}