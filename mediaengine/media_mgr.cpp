/*
 * Copyright 2025 LiveKit, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "media_mgr.h"

#include "media.h"
#include "fallback_capture.h"

#include <QDebug>

#include <livekit/livekit.h>

MediaMgr::MediaMgr() = default;

MediaMgr::~MediaMgr() {
    stopMic();
    stopCamera();
    stopSpeaker();
    shutdownRenderer();
}

bool MediaMgr::ensureSDLInit(Uint32 flags) {
    if ((SDL_WasInit(flags) & flags) == flags) {
        return true; // already init
    }
    if (!SDL_InitSubSystem(flags)) {
        qCritical() << "SDL_InitSubSystem failed (flags=" << flags << "):"
                    << SDL_GetError();
        return false;
    }
    return true;
}

// ---------- Mic control ----------

bool MediaMgr::startMic(const std::shared_ptr<livekit::AudioSource> &audio_source) {
    stopMic();

    if (!audio_source) {
        qCritical() << "startMic: audioSource is null";
        return false;
    }

    mic_source_ = audio_source;
    mic_running_.store(true, std::memory_order_relaxed);

    // Try SDL path
    if (!ensureSDLInit(SDL_INIT_AUDIO)) {
        qWarning() << "No SDL audio, falling back to noise loop.";
        mic_using_ = false;
        mic_thread_ = std::thread(runNoiseCaptureLoop, mic_source_, std::ref(mic_running_));
        return true;
    }

    int recCount = 0;
    SDL_AudioDeviceID *recDevs = SDL_GetAudioRecordingDevices(&recCount);
    if (!recDevs || recCount == 0) {
        qWarning() << "No microphone devices found, falling back to noise loop.";
        if (recDevs)
            SDL_free(recDevs);
        mic_using_ = false;
        mic_thread_ = std::thread(runNoiseCaptureLoop, mic_source_, std::ref(mic_running_));
        return true;
    }

    SDL_free(recDevs);

    // We have at least one mic; use SDL
    mic_using_ = true;

    mic_ = std::make_unique<MicSource>(
        mic_source_->sample_rate(),
        mic_source_->num_channels(),
        mic_source_->sample_rate() / 100, // ~10ms
        [src = mic_source_](const int16_t *samples,
                            int num_samples_per_channel,
                            int sample_rate, int num_channels) {
            livekit::AudioFrame frame = livekit::AudioFrame::create(sample_rate, num_channels,
                                                                    num_samples_per_channel);
            std::memcpy(frame.data().data(), samples,
                        num_samples_per_channel * num_channels * sizeof(int16_t));
            try {
                src->captureFrame(frame);
            }
            catch (const std::exception &e) {
                qCritical() << "Error in captureFrame (SDL mic):" << e.what();
            }
        });

    if (!mic_->init()) {
        qWarning() << "Failed to init SDL mic, falling back to noise loop.";
        mic_using_ = false;
        mic_.reset();
        mic_thread_ = std::thread(runNoiseCaptureLoop, mic_source_, std::ref(mic_running_));
        return true;
    }

    mic_thread_ = std::thread(&MediaMgr::micLoopSDL, this);
    return true;
}

void MediaMgr::micLoopSDL() {
    while (mic_running_.load(std::memory_order_relaxed)) {
        mic_->pump();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void MediaMgr::stopMic() {
    mic_running_.store(false, std::memory_order_relaxed);
    if (mic_thread_.joinable()) {
        mic_thread_.join();
    }
    mic_.reset();
    mic_source_.reset();
}

// ---------- Camera control ----------

bool MediaMgr::startCamera(const std::shared_ptr<livekit::VideoSource> &video_source) {
    stopCamera();

    if (!video_source) {
        qCritical() << "startCamera: videoSource is null";
        return false;
    }

    cam_source_ = video_source;
    cam_running_.store(true, std::memory_order_relaxed);

    // Try SDL
    if (!ensureSDLInit(SDL_INIT_CAMERA)) {
        qWarning() << "No SDL camera subsystem, using fake video loop.";
        cam_using_ = false;
        cam_thread_ = std::thread(runFakeVideoCaptureLoop, cam_source_, std::ref(cam_running_));
        return true;
    }

    int camCount = 0;
    SDL_CameraID *cams = SDL_GetCameras(&camCount);
    if (!cams || camCount == 0) {
        qWarning() << "No camera devices found, using fake video loop.";
        if (cams)
            SDL_free(cams);
        cam_using_ = false;
        cam_thread_ = std::thread(runFakeVideoCaptureLoop, cam_source_, std::ref(cam_running_));
        return true;
    }

    SDL_free(cams);

    cam_using_ = true;
    cam_ = std::make_unique<CamSource>(
        1280, 720, 30,
        SDL_PIXELFORMAT_RGBA32, // Note SDL_PIXELFORMAT_RGBA8888 is not compatable
        // with Livekit RGBA format.
        [src = cam_source_](const uint8_t *pixels, int pitch, int width,
                            int height, SDL_PixelFormat /*fmt*/,
                            Uint64 timestampNS) {
            auto frame = livekit::VideoFrame::create(width, height, livekit::VideoBufferType::RGBA);
            uint8_t *dst = frame.data();
            const int dstPitch = width * 4;

            for (int y = 0; y < height; ++y) {
                std::memcpy(dst + y * dstPitch, pixels + y * pitch, dstPitch);
            }

            try {
                src->captureFrame(frame, timestampNS / 1000, livekit::VideoRotation::VIDEO_ROTATION_0);
            } catch (const std::exception &e) {
                qCritical() << "Error in captureFrame (SDL cam):" << e.what();
            }
        });

    if (!cam_->init()) {
        qWarning() << "Failed to init SDL camera, using fake video loop.";
        cam_using_ = false;
        cam_.reset();
        cam_thread_ = std::thread(runFakeVideoCaptureLoop, cam_source_, std::ref(cam_running_));
        return true;
    }

    cam_thread_ = std::thread(&MediaMgr::cameraLoopSDL, this);
    return true;
}

void MediaMgr::cameraLoopSDL() {
    while (cam_running_.load(std::memory_order_relaxed)) {
        cam_->pump();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void MediaMgr::stopCamera() {
    cam_running_.store(false, std::memory_order_relaxed);
    if (cam_thread_.joinable()) {
        cam_thread_.join();
    }
    cam_.reset();
    cam_source_.reset();
}

// ---------- Speaker control (placeholder) ----------

bool MediaMgr::startSpeaker(const std::shared_ptr<livekit::AudioStream> &audio_stream) {
    stopSpeaker();

    if (!audio_stream) {
        qCritical() << "startSpeaker: audioStream is null";
        return false;
    }

    if (!ensureSDLInit(SDL_INIT_AUDIO)) {
        qCritical() << "startSpeaker: SDL_INIT_AUDIO failed";
        return false;
    }

    speaker_stream_ = audio_stream;
    speaker_running_.store(true, std::memory_order_relaxed);

    // Note, we don't open the speaker since the format is unknown yet.
    // Instead, open the speaker in the speakerLoopSDL thread with the native
    // format.
    try {
        speaker_thread_ = std::thread(&MediaMgr::speakerLoopSDL, this);
    } catch (const std::exception &e) {
        qCritical() << "startSpeaker: failed to start speaker thread:" << e.what();
        speaker_running_.store(false, std::memory_order_relaxed);
        speaker_stream_.reset();
        return false;
    }

    return true;
}

void MediaMgr::speakerLoopSDL() {
    SDL_AudioStream *localStream = nullptr;
    SDL_AudioDeviceID dev = 0;

    while (speaker_running_.load(std::memory_order_relaxed)) {
        if (!speaker_stream_) {
            break;
        }

        livekit::AudioFrameEvent ev;
        if (!speaker_stream_->read(ev)) {
            // EOS or closed
            break;
        }

        const livekit::AudioFrame &frame = ev.frame;
        const auto &data = frame.data();
        if (data.empty()) {
            continue;
        }

        // Lazily open SDL audio stream based on the first frame's format, so no
        // resampler is needed.
        if (!localStream) {
            SDL_AudioSpec want{};
            want.format = SDL_AUDIO_S16;
            want.channels = static_cast<Uint8>(frame.num_channels());
            want.freq = frame.sample_rate();

            localStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &want,
                                          /*callback=*/nullptr,
                                          /*userdata=*/nullptr);

            if (!localStream) {
                qCritical() << "speakerLoopSDL: SDL_OpenAudioDeviceStream failed:"
                            << SDL_GetError();
                break;
            }

            audio_stream_ = localStream; // store if you want to inspect later

            dev = SDL_GetAudioStreamDevice(localStream);
            if (dev == 0) {
                qCritical() << "speakerLoopSDL: SDL_GetAudioStreamDevice failed:"
                            << SDL_GetError();
                break;
            }

            if (!SDL_ResumeAudioDevice(dev)) {
                qCritical() << "speakerLoopSDL: SDL_ResumeAudioDevice failed:"
                            << SDL_GetError();
                break;
            }
        }

        // Push PCM to SDL. We assume frames are already S16, interleaved, matching
        // sample_rate / channels we used above.
        const int numBytes = static_cast<int>(data.size() * sizeof(std::int16_t));

        if (!SDL_PutAudioStreamData(localStream, data.data(), numBytes)) {
            qCritical() << "speakerLoopSDL: SDL_PutAudioStreamData failed:"
                        << SDL_GetError();
            break;
        }

        // Tiny sleep to avoid busy loop; SDL buffers internally.
        SDL_Delay(2);
    }

    if (localStream) {
        SDL_DestroyAudioStream(localStream);
        localStream = nullptr;
        audio_stream_ = nullptr;
    }

    speaker_running_.store(false, std::memory_order_relaxed);
}

void MediaMgr::stopSpeaker() {
    speaker_running_.store(false, std::memory_order_relaxed);
    if (speaker_thread_.joinable()) {
        speaker_thread_.join();
    }
    if (audio_stream_) {
        SDL_DestroyAudioStream(audio_stream_);
        audio_stream_ = nullptr;
    }
    speaker_stream_.reset();
}

// ---------- Renderer control (placeholder) ----------

bool MediaMgr::initRenderer(const std::shared_ptr<livekit::VideoStream> &video_stream) {
    if (!video_stream) {
        qCritical() << "startRenderer: videoStream is null";
        return false;
    }

    shutdownRenderer();

    renderer_stream_ = video_stream;
    renderer_running_.store(true, std::memory_order_relaxed);

    try {
        renderer_thread_ = std::thread(&MediaMgr::renderLoopSDL, this);
    } catch (const std::exception &e) {
        qCritical() << "startRenderer: failed to start renderer thread:" << e.what();
        renderer_running_.store(false, std::memory_order_relaxed);
        renderer_stream_.reset();
        return false;
    }

    return true;
}

void MediaMgr::shutdownRenderer() {
    renderer_running_.store(false, std::memory_order_relaxed);

    if (renderer_thread_.joinable()) {
        renderer_thread_.join();
    }

    renderer_stream_.reset();

    std::lock_guard<std::mutex> lock(renderer_frame_mutex_);
    latest_video_rgba_.clear();
    latest_video_width_ = 0;
    latest_video_height_ = 0;
}

void MediaMgr::renderLoopSDL() {
    while (renderer_running_.load(std::memory_order_relaxed)) {
        if (!renderer_stream_) {
            break;
        }

        livekit::VideoFrameEvent vfe;
        if (!renderer_stream_->read(vfe)) {
            break;
        }

        livekit::VideoFrame &frame = vfe.frame;
        if (frame.type() != livekit::VideoBufferType::RGBA) {
            try {
                frame = frame.convert(livekit::VideoBufferType::RGBA, false);
            } catch (const std::exception &ex) {
                qCritical() << "renderLoopSDL: convert to RGBA failed:" << ex.what();
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
            std::lock_guard<std::mutex> lock(renderer_frame_mutex_);
            latest_video_rgba_ = std::move(rgba);
            latest_video_width_ = width;
            latest_video_height_ = height;
        }
    }
}

bool MediaMgr::copyLatestVideoFrame(std::vector<std::uint8_t> &rgba, int &width, int &height) {
    std::lock_guard<std::mutex> lock(renderer_frame_mutex_);
    if (latest_video_rgba_.empty() || latest_video_width_ <= 0 || latest_video_height_ <= 0) {
        return false;
    }

    rgba = latest_video_rgba_;
    width = latest_video_width_;
    height = latest_video_height_;
    return true;
}