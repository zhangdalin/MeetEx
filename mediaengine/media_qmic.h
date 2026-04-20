#ifndef MEDIA_QMIC_H
#define MEDIA_QMIC_H

#include <QAudioDevice>
#include <QAudioSource>
#include <QIODevice>
#include <QObject>

#include <functional>
#include <memory>

class QMicSource : public QObject {
    Q_OBJECT

public:
    using AudioCallback = std::function<void(
        const int16_t *samples, // interleaved
        int num_samples_per_channel, int sample_rate, int num_channels)>;

    QMicSource(int sample_rate = 48000, int channels = 1,
              int frame_samples = 480, AudioCallback cb = nullptr);

    ~QMicSource();

    // Initialize Qt audio source for recording.
    bool init();

    // Call regularly to pull mic data and send to callback.
    void pump();

    void pause();
    void resume();

    bool isValid() const { return audio_source_ != nullptr && device_ != nullptr; }

private:
    std::unique_ptr<QAudioSource> audio_source_;
    QIODevice *device_ = nullptr;
    int sample_rate_;
    int channels_;
    int frame_samples_;
    AudioCallback callback_;
};

#endif // MEDIA_QMIC_H