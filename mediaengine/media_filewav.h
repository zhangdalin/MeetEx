#ifndef MEDIA_FILEWAV_H
#define MEDIA_FILEWAV_H

#include "livekit/livekit.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

// Simple WAV container for 16-bit PCM files
struct WavData {
    int sample_rate = 0;
    int num_channels = 0;
    std::vector<int16_t> samples;
};

// Helper that loads 16-bit PCM WAV (16-bit, PCM only)
WavData loadWav16(const std::string &path);

class WavSource {
public:
    // loop_enabled: whether to loop when reaching the end
    WavSource(const std::string &path, int expected_sample_rate,
                   int expected_channels, bool loop_enabled = true);

    // Fill a frame with the next chunk of audio.
    void fillFrame(livekit::AudioFrame &frame);

private:
    void initLoopDelayCounter();

    WavData wav_;
    std::size_t playhead_ = 0;

    const bool loop_enabled_;
    int sample_rate_;
    int num_channels_;
};

#endif // MEDIA_FILEWAV_H