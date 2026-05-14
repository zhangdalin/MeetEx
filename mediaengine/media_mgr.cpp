#include "media_mgr.h"
#include "media_def.h"
#include "media_qmic.h"
#include "media_qcam.h"
#include "media_qspk.h"
#include "media_filewav.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstring>

#include <QCameraDevice>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QGuiApplication>
#include <QScreen>
#include <QImage>
#include <QTimer>
#include <QPixmap>
#include <QDateTime>

#include <QDebug>
#include <QThread>

#include <livekit/livekit.h>

namespace {

constexpr float kSilenceDb = -100.0f;
constexpr float kLevelFloorDb = -60.0f;
constexpr float kSpeakingOnDb = -42.0f;
constexpr float kSpeakingOffDb = -48.0f;
constexpr float kSmoothingAlpha = 0.35f;

AudioLevelInfo analyzeAudioLevel(const int16_t *samples,
                                 std::size_t sample_count,
                                 const AudioLevelInfo &previous) {
    AudioLevelInfo current = previous;
    if (!samples || sample_count == 0) {
        current.valid = false;
        current.rms = 0.0f;
        current.peak = 0.0f;
        current.db = kSilenceDb;
        current.smoothed_db = previous.valid
            ? (1.0f - kSmoothingAlpha) * previous.smoothed_db + kSmoothingAlpha * kSilenceDb
            : kSilenceDb;
        current.level = std::clamp((current.smoothed_db - kLevelFloorDb) / -kLevelFloorDb, 0.0f, 1.0f);
        current.speaking = previous.speaking ? (current.smoothed_db >= kSpeakingOffDb)
                                             : (current.smoothed_db >= kSpeakingOnDb);
        return current;
    }

    double sum_squares = 0.0;
    int peak_abs = 0;
    for (std::size_t i = 0; i < sample_count; ++i) {
        const int value = static_cast<int>(samples[i]);
        const int abs_value = std::abs(value);
        peak_abs = std::max(peak_abs, abs_value);

        const double normalized = static_cast<double>(value) / 32768.0;
        sum_squares += normalized * normalized;
    }

    current.valid = true;
    current.rms = static_cast<float>(std::sqrt(sum_squares / static_cast<double>(sample_count)));
    current.peak = static_cast<float>(peak_abs) / 32767.0f;
    current.db = current.rms > 1e-9f ? static_cast<float>(20.0 * std::log10(current.rms)) : kSilenceDb;
    current.smoothed_db = previous.valid
        ? (1.0f - kSmoothingAlpha) * previous.smoothed_db + kSmoothingAlpha * current.db
        : current.db;
    current.level = std::clamp((current.smoothed_db - kLevelFloorDb) / -kLevelFloorDb, 0.0f, 1.0f);
    current.speaking = previous.speaking ? (current.smoothed_db >= kSpeakingOffDb)
                                         : (current.smoothed_db >= kSpeakingOnDb);
    return current;
}

} // namespace

MediaMgr::MediaMgr() = default;

MediaMgr::~MediaMgr() {
    stopMic();
    stopCamera();
    stopScreenShare();
    stopAllPlayback();
    stopAllRenders();
}

// ---------- Mic control ----------

bool MediaMgr::startMic(const std::shared_ptr<livekit::AudioSource> &audio_source) {
    stopMic();

    if (!audio_source) {
        qCritical() << QThread::currentThread() << __FUNCTION__ << "audioSource is null";
        return false;
    }

    mic_source_ = audio_source;
    mic_running_.store(true, std::memory_order_relaxed);

    if (QMediaDevices::audioInputs().isEmpty()) {
        qWarning() << QThread::currentThread() << __FUNCTION__ << "No microphone devices found, falling back to noise loop.";
        mic_using_ = false;
        mic_thread_ = std::thread(&MediaMgr::runNoiseCapLoop, this, mic_source_, std::ref(mic_running_));
        return true;
    }

    mic_using_ = true;
    mic_thread_ = std::thread(&MediaMgr::micLoop, this, mic_source_, std::ref(mic_running_));
    return true;
}

// Test utils to run a capture loop to publish noisy audio frames to the room
void MediaMgr::runNoiseCapLoop(const std::shared_ptr<livekit::AudioSource> &source,
                         std::atomic<bool> &running_flag) {
    const int sample_rate = source->sample_rate();
    const int num_channels = source->num_channels();
    const int frame_ms = 10;
    const int samples_per_channel = sample_rate * frame_ms / 1000;

    // FIX: variable name should not shadow the type
    WavSource wavSource("data/welcome.wav", 48000, 1, false);

    using Clock = std::chrono::steady_clock;
    auto next_deadline = Clock::now();
    while (running_flag.load(std::memory_order_relaxed)) {
        livekit::AudioFrame frame =
            livekit::AudioFrame::create(sample_rate, num_channels, samples_per_channel);
        wavSource.fillFrame(frame);
        updateLocalAudioLevel(frame.data().data(), frame.data().size());
        try {
            source->captureFrame(frame);
        } catch (const std::exception &e) {
            qCritical() << QThread::currentThread() << __FUNCTION__ << "Error in captureFrame (noise):" << e.what();
            break;
        }

        // Pace the loop to roughly real-time
        next_deadline += std::chrono::milliseconds(frame_ms);
        std::this_thread::sleep_until(next_deadline);
    }

    try {
        source->clearQueue();
    } catch (...) {
        qWarning() << QThread::currentThread() << __FUNCTION__ << "Error in clearQueue (noise)";
    }
}

void MediaMgr::micLoop(const std::shared_ptr<livekit::AudioSource> &source,
                        std::atomic<bool> &running_flag) {
    QMicSource mic(source->sample_rate(),
                source->num_channels(),
                source->sample_rate() / 100,
                [this, source](const int16_t *samples,
                        int num_samples_per_channel,
                        int sample_rate, int num_channels) {
                    updateLocalAudioLevel(samples,
                                          static_cast<std::size_t>(num_samples_per_channel) *
                                              static_cast<std::size_t>(num_channels));
                    livekit::AudioFrame frame = livekit::AudioFrame::create(sample_rate, num_channels,
                                                                            num_samples_per_channel);
                    std::memcpy(frame.data().data(), samples,
                                num_samples_per_channel * num_channels * sizeof(int16_t));
                    try {
                        source->captureFrame(frame);
                    } catch (const std::exception &e) {
                        qCritical() << QThread::currentThread() << __FUNCTION__ << "Error in captureFrame (Qt mic):" << e.what();
                    }
                });

    if (!mic.init()) {
        qWarning() << QThread::currentThread() << __FUNCTION__ << "Failed to init Qt mic, falling back to noise loop.";
        mic_thread_ = std::thread(&MediaMgr::runNoiseCapLoop, this, mic_source_, std::ref(mic_running_));
        return;
    }
    while (running_flag.load(std::memory_order_relaxed)) {
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
    resetLocalAudioLevel();
}

// ---------- Camera control ----------

bool MediaMgr::startCamera(const std::shared_ptr<livekit::VideoSource> &video_source, const std::string &track_sid) {
    stopCamera();

    if (!video_source) {
        qCritical() << QThread::currentThread() << __FUNCTION__ << "videoSource is null";
        return false;
    }

    if (track_sid.empty()) {
        qCritical() << QThread::currentThread() << __FUNCTION__ << "track_sid is empty";
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
        qWarning() << QThread::currentThread() << __FUNCTION__ << "No camera devices found, using fake video loop.";
        cam_using_ = false;
        cam_thread_ = std::thread(&MediaMgr::runFakeVideoCapLoop, this, cam_source_, 
            std::ref(worker->frameBuff), std::ref(cam_running_));
        return true;
    }

    cam_ = std::make_unique<QCamSource>(
        VIDEO_WIDTH, VIDEO_HEIGHT, VIDEO_FPS,
        [source = cam_source_, buff = &(worker->frameBuff)](const uint8_t *pixels, int pitch, int width,
                            int height, int64_t timestampNs) {
            auto frame = livekit::VideoFrame::create(width, height, livekit::VideoBufferType::RGBA);
            uint8_t *dst = frame.data();
            const int dstPitch = width * 4;

            for (int y = 0; y < height; ++y) {
                std::memcpy(dst + y * dstPitch, pixels + y * pitch, dstPitch);
            }

            // add frame to local render (reuse existing buffer, reallocate only on resolution change)
            if (buff) {
                std::lock_guard<std::mutex> lock(buff->mutex);
                const size_t rgba_size = static_cast<size_t>(width * height * 4);
                if (buff->rgba.size() != rgba_size)
                    buff->rgba.resize(rgba_size);
                std::memcpy(buff->rgba.data(), frame.data(), rgba_size);
                buff->width = width;
                buff->height = height;
            }

            // add frame to video source;
            try {
                source->captureFrame(frame, timestampNs / 1000, livekit::VideoRotation::VIDEO_ROTATION_0);
            } catch (const std::exception &e) {
                qCritical() << QThread::currentThread() << __FUNCTION__ << "Error in captureFrame (Qt cam):" << e.what();
            }
        });

    cam_using_ = true;

    if (!cam_->init()) {
        qWarning() << QThread::currentThread() << __FUNCTION__ << "Failed to init Qt camera, using fake video loop.";
        cam_using_ = false;
        cam_.reset();
        cam_thread_ = std::thread(&MediaMgr::runFakeVideoCapLoop, this, cam_source_, 
            std::ref(worker->frameBuff), std::ref(cam_running_));
        return true;
    }

    // QCamSource delivers frames via Qt's event system; no pump thread needed.
    return true;
}


// Fake video source: solid color cycling
void MediaMgr::runFakeVideoCapLoop(const std::shared_ptr<livekit::VideoSource> &source,
                            VideoFrameBuff& frameBuff,
                            std::atomic<bool> &running_flag) {
    auto frame = livekit::VideoFrame::create(VIDEO_WIDTH, VIDEO_HEIGHT, livekit::VideoBufferType::BGRA);
    const double framerate = 1.0 / VIDEO_FPS;

    // Pre-allocate render buffer once; no heap allocation inside the loop
    {
        std::lock_guard<std::mutex> lock(frameBuff.mutex);
        frameBuff.rgba.resize(static_cast<size_t>(VIDEO_WIDTH * VIDEO_HEIGHT * 4));
        frameBuff.width = VIDEO_WIDTH;
        frameBuff.height = VIDEO_HEIGHT;
    }

    while (running_flag.load(std::memory_order_relaxed)) {
        static auto start = std::chrono::high_resolution_clock::now();
        float t = std::chrono::duration<float>(
                      std::chrono::high_resolution_clock::now() - start)
                      .count();
        // Cycle every 4 seconds: 0=red, 1=green, 2=blue, 3=black
        int stage = static_cast<int>(t) % 4;

        std::array<uint8_t, 4> rgb{};
        switch (stage) {
        case 0: // red
            rgb = {255, 0, 0, 0};
            break;
        case 1: // green
            rgb = {0, 255, 0, 0};
            break;
        case 2: // blue
            rgb = {0, 0, 255, 0};
            break;
        case 3: // black
        default:
            rgb = {0, 0, 0, 0};
            break;
        }

        // ARGB
        uint8_t *data = frame.data();
        const size_t size = frame.dataSize();
        for (size_t i = 0; i < size; i += 4) {
            data[i + 0] = 255;    // A
            data[i + 1] = rgb[0]; // R
            data[i + 2] = rgb[1]; // G
            data[i + 3] = rgb[2]; // B
        }

        // add frame to local render (direct write into pre-allocated buffer)
        {
            std::lock_guard<std::mutex> lock(frameBuff.mutex);
            std::memcpy(frameBuff.rgba.data(), frame.data(), frameBuff.rgba.size());
        }

        // add frame to video source;
        try {
            // If VideoSource is ARGB-capable, pass frame.
            // If it expects I420, pass i420 instead.
            source->captureFrame(frame, 0, livekit::VideoRotation::VIDEO_ROTATION_0);
        } catch (const std::exception &e) {
            qCritical() << QThread::currentThread() << __FUNCTION__ << "Error in captureFrame (fake video):" << e.what();
            break;
        }

        std::this_thread::sleep_for(std::chrono::duration<double>(framerate));
    }
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

bool MediaMgr::startScreenShare(const std::shared_ptr<livekit::VideoSource> &video_source, const std::string &track_sid) {
    stopScreenShare();

    if (!video_source) {
        qCritical() << QThread::currentThread() << __FUNCTION__ << "videoSource is null";
        return false;
    }

    if (track_sid.empty()) {
        qCritical() << QThread::currentThread() << __FUNCTION__ << "track_sid is empty";
        return false;
    }

    if (!QGuiApplication::primaryScreen()) {
        qCritical() << QThread::currentThread() << __FUNCTION__ << "No available screen for capture";
        return false;
    }

    screen_source_ = video_source;
    screen_track_sid_ = track_sid;
    screen_using_ = true;

    screen_worker_.reset(new QObject(qApp));
    screen_timer_.reset(new QTimer(screen_worker_.get()));
    screen_timer_->setTimerType(Qt::CoarseTimer);
    QObject::connect(screen_timer_.get(), &QTimer::timeout, screen_worker_.get(), [this]() {
        captureScreenFrame();
    });

    const int intervalMs = 1000 / 10;
    screen_timer_->start(intervalMs);
    qInfo() << QThread::currentThread() << __FUNCTION__ << "screen share started, interval=" << intervalMs;
    return true;
}

void MediaMgr::stopScreenShare() {
    if (screen_timer_) {
        screen_timer_->stop();
    }
    screen_timer_.reset();
    screen_worker_.reset();
    screen_source_.reset();
    screen_track_sid_.clear();
    screen_using_ = false;
}

void MediaMgr::captureScreenFrame() {
    if (!screen_source_) {
        return;
    }

    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) {
        qWarning() << QThread::currentThread() << __FUNCTION__ << "primary screen is unavailable";
        return;
    }

    QPixmap pixmap = screen->grabWindow(0);
    const QImage image = pixmap.toImage().convertToFormat(QImage::Format_RGBA8888);
    const int width = image.width();
    const int height = image.height();
    if (width <= 0 || height <= 0) {
        qWarning() << QThread::currentThread() << __FUNCTION__ << "captured invalid screen image";
        return;
    }

    auto frame = livekit::VideoFrame::create(width, height, livekit::VideoBufferType::RGBA);
    std::memcpy(frame.data(), image.constBits(), static_cast<size_t>(width) * static_cast<size_t>(height) * 4);

    const qint64 timestampNs = QDateTime::currentMSecsSinceEpoch() * 1000000LL;
    try {
        screen_source_->captureFrame(frame, timestampNs / 1000, livekit::VideoRotation::VIDEO_ROTATION_0);
    } catch (const std::exception &e) {
        qCritical() << QThread::currentThread() << __FUNCTION__ << "Error in captureFrame (screen share):" << e.what();
    }
}

// ---------- Speaker control ----------

bool MediaMgr::startPlayback(const std::shared_ptr<livekit::AudioStream> &audio_stream, const std::string& track_sid) {
    if (!audio_stream) {
        qCritical() << QThread::currentThread() << __FUNCTION__ << "audioStream is null";
        return false;
    }

    if (track_sid.empty()) {
        qCritical() << QThread::currentThread() << __FUNCTION__ << "track_sid is empty";
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
    resetRemoteAudioLevel(track_sid);

    // Start the shared mixer output thread exactly once, before playbackLoop begins pushing data
    {
        bool expected = false;
        if (mix_running_.compare_exchange_strong(expected, true)) {
            mix_thread_ = std::thread(&MediaMgr::mixLoop, this);
        }
    }

    try {
        worker->thread = std::thread(&MediaMgr::playbackLoop, this, track_sid, worker);
    } catch (const std::exception &e) {
        qCritical() << QThread::currentThread() << __FUNCTION__
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

        updateRemoteAudioLevel(track_sid, data.data(), data.size());

        // Push decoded PCM into the shared mixer buffer for this track
        {
            std::lock_guard<std::mutex> lk(mix_mutex_);
            auto &mt = mix_tracks_[track_sid];
            const int total = frame.samples_per_channel() * frame.num_channels();
            mt.buf.insert(mt.buf.end(), data.data(), data.data() + total);
        }
        mix_cv_.notify_one();
    }

    // Remove this track from the mixer when the stream ends
    {
        std::lock_guard<std::mutex> lk(mix_mutex_);
        mix_tracks_.erase(track_sid);
    }
    resetRemoteAudioLevel(track_sid);

    worker->running.store(false, std::memory_order_relaxed);

    {
        std::lock_guard<std::mutex> lock(playback_mutex_);
        auto it = playback_.find(track_sid);
        if (it != playback_.end() && it->second == worker) {
            playback_.erase(it);
        }
    }
}

void MediaMgr::stopPlayback(const std::string& track_sid) {
    std::shared_ptr<PlaybackWorker> worker = nullptr;
    {
        std::lock_guard<std::mutex> lock(playback_mutex_);
        auto it = playback_.find(track_sid);
        if (it != playback_.end()) {
            worker = it->second;
            playback_.erase(it);
        }
    }

    if (worker) {
        worker->running.store(false, std::memory_order_relaxed);
        if (worker->thread.joinable()) {
            worker->thread.join();
        }
        worker->stream.reset();
    }

    // Remove this track from the mixer
    {
        std::lock_guard<std::mutex> lk(mix_mutex_);
        mix_tracks_.erase(track_sid);
    }
    resetRemoteAudioLevel(track_sid);
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

    // Stop the mixer output thread
    mix_running_.store(false, std::memory_order_relaxed);
    mix_cv_.notify_all();
    if (mix_thread_.joinable()) {
        mix_thread_.join();
    }
    {
        std::lock_guard<std::mutex> lk(mix_mutex_);
        mix_tracks_.clear();
    }
    resetAllRemoteAudioLevels();
}

AudioLevelInfo MediaMgr::localAudioLevel() const {
    std::lock_guard<std::mutex> lock(audio_levels_mutex_);
    return local_audio_level_;
}

bool MediaMgr::isLocalAudioSpeaking() const {
    return localAudioLevel().speaking;
}

AudioLevelInfo MediaMgr::remoteAudioLevel(const std::string &track_sid) const {
    std::lock_guard<std::mutex> lock(audio_levels_mutex_);
    auto it = remote_audio_levels_.find(track_sid);
    if (it == remote_audio_levels_.end()) {
        return {};
    }
    return it->second;
}

bool MediaMgr::isRemoteAudioSpeaking(const std::string &track_sid) const {
    return remoteAudioLevel(track_sid).speaking;
}

std::unordered_map<std::string, AudioLevelInfo> MediaMgr::remoteAudioLevels() const {
    std::lock_guard<std::mutex> lock(audio_levels_mutex_);
    return remote_audio_levels_;
}

void MediaMgr::updateLocalAudioLevel(const int16_t *samples, std::size_t sample_count) {
    std::lock_guard<std::mutex> lock(audio_levels_mutex_);
    local_audio_level_ = analyzeAudioLevel(samples, sample_count, local_audio_level_);
}

void MediaMgr::updateRemoteAudioLevel(const std::string &track_sid,
                                      const int16_t *samples,
                                      std::size_t sample_count) {
    std::lock_guard<std::mutex> lock(audio_levels_mutex_);
    auto &level = remote_audio_levels_[track_sid];
    level = analyzeAudioLevel(samples, sample_count, level);
}

void MediaMgr::resetLocalAudioLevel() {
    std::lock_guard<std::mutex> lock(audio_levels_mutex_);
    local_audio_level_ = {};
}

void MediaMgr::resetRemoteAudioLevel(const std::string &track_sid) {
    std::lock_guard<std::mutex> lock(audio_levels_mutex_);
    remote_audio_levels_.erase(track_sid);
}

void MediaMgr::resetAllRemoteAudioLevels() {
    std::lock_guard<std::mutex> lock(audio_levels_mutex_);
    remote_audio_levels_.clear();
}

// ---------- Renderer control (placeholder) ----------

bool MediaMgr::startRender(const std::shared_ptr<livekit::VideoStream> &video_stream,
                             const std::string &track_sid) {
    if (!video_stream) {
        qCritical() << QThread::currentThread() << __FUNCTION__ << "videoStream is null";
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
        qCritical() << QThread::currentThread() << __FUNCTION__ 
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

void MediaMgr::stopRender(const std::string &track_sid) {
    std::shared_ptr<RenderWorker> worker = nullptr;
    {
        std::lock_guard<std::mutex> lock(renders_mutex_);
        auto it = renders_.find(track_sid);
        if (it != renders_.end()) {
            worker = it->second;
            renders_.erase(it);
        }
    }

    if (worker) {
        worker->running.store(false, std::memory_order_relaxed);
        if (worker->thread.joinable()) {
            worker->thread.join();
        }
        worker->stream.reset();
    }
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
    std::vector<std::uint8_t> rgba; // persistent; swapped with frameBuff each frame to avoid realloc
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
                qCritical() << QThread::currentThread() << __FUNCTION__ << "convert to RGBA failed:" << ex.what();
                continue;
            }
        }

        const int width = frame.width();
        const int height = frame.height();
        if (width <= 0 || height <= 0) {
            continue;
        }

        const size_t rgba_size = static_cast<size_t>(width * height * 4);
        rgba.resize(rgba_size); // no-op when resolution is unchanged
        std::memcpy(rgba.data(), frame.data(), rgba_size);

        {
            std::lock_guard<std::mutex> lock(worker->frameBuff.mutex);
            rgba.swap(worker->frameBuff.rgba); // swap: rgba recycles the old buffer for next frame
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

    // resize + memcpy: avoids reallocation when caller reuses the same frameBuff
    const size_t sz = worker->frameBuff.rgba.size();
    frameBuff.rgba.resize(sz);
    std::memcpy(frameBuff.rgba.data(), worker->frameBuff.rgba.data(), sz);
    frameBuff.width = worker->frameBuff.width;
    frameBuff.height = worker->frameBuff.height;
    return true;
}

// ---------- Audio mixer output ----------

void MediaMgr::mixLoop() {
    // Fixed output format: 48 kHz mono (standard WebRTC format).
    // All remote tracks are assumed to share this format.
    static constexpr int kSampleRate   = 48000;
    static constexpr int kChannels     = 1;
    static constexpr int kFrameSamples = kSampleRate / 100; // 10 ms = 480 samples

    QSpkSink sink(kSampleRate, kChannels);
    if (!sink.init()) {
        qCritical() << QThread::currentThread() << __FUNCTION__ << "Failed to init mix sink";
        mix_running_.store(false, std::memory_order_relaxed);
        return;
    }

    std::vector<int32_t> acc;
    std::vector<int16_t> out;

    using Clock = std::chrono::steady_clock;
    auto next_deadline = Clock::now() + std::chrono::milliseconds(10);

    while (mix_running_.load(std::memory_order_relaxed)) {
        // Tick at a fixed 10 ms rate regardless of how often playbackLoop notifies.
        // This prevents bursting too much data into QAudioSink at once.
        std::this_thread::sleep_until(next_deadline);
        next_deadline += std::chrono::milliseconds(10);

        if (!mix_running_.load(std::memory_order_relaxed))
            break;

        size_t maxContrib = 0;
        {
            std::lock_guard<std::mutex> lk(mix_mutex_);

            // Drain up to kFrameSamples from every active track and sum them
            const size_t take = static_cast<size_t>(kFrameSamples);
            acc.assign(take, 0);

            for (auto &[_, mt] : mix_tracks_) {
                const size_t avail = std::min(mt.buf.size(), take);
                if (avail == 0)
                    continue;
                for (size_t i = 0; i < avail; ++i)
                    acc[i] += static_cast<int32_t>(mt.buf[i]);
                if (avail > maxContrib)
                    maxContrib = avail;
                mt.buf.erase(mt.buf.begin(),
                             mt.buf.begin() + static_cast<ptrdiff_t>(avail));
            }
        } // release mix_mutex_ before writing to the sink

        if (maxContrib == 0)
            continue;

        // Saturating cast: clamp to int16 range to prevent wrap-around distortion
        out.resize(maxContrib);
        for (size_t i = 0; i < maxContrib; ++i) {
            int32_t v = acc[i];
            if (v >  32767) v =  32767;
            if (v < -32768) v = -32768;
            out[i] = static_cast<int16_t>(v);
        }

        sink.enqueue(out.data(), static_cast<int>(out.size() / static_cast<size_t>(kChannels)));
    }
}