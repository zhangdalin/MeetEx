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

#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_camera.h>

#include "wav_audio_source.h"

#include <atomic>
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

// Forward-declared SDL helpers (you can also keep these separate if you like)
class MicSource;
class QCamSource;

struct VideoFrameBuff {
    std::mutex mutex;
    std::vector<uint8_t> rgba;
    int width = 0;
    int height = 0;
};

// SDLMediaManager gives you dedicated control over:
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
    // ---- SDL bootstrap helpers ----
    bool ensureSDLInit(Uint32 flags);

    // ---- Mic helpers ----
    void micLoopSDL();
    void micLoopNoise();

    // ---- Camera helpers ----
    void cameraLoopFake();

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

    // ---- Playback helpers ----
    void playbackLoopSDL(const std::string &track_sid, const std::shared_ptr<PlaybackWorker> &worker);
    void renderLoop(const std::string &track_sid, const std::shared_ptr<RenderWorker> &worker);

    // Mic
    std::shared_ptr<livekit::AudioSource> mic_source_;
    std::unique_ptr<MicSource> mic_;
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
