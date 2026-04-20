#ifndef MEDIA_QSPKSINK_H
#define MEDIA_QSPKSINK_H

#include <QObject>
#include <QAudioSink>

#include <memory>

// -------------------------
// QtQSpkSink
// -------------------------
// For remote audio: when you get a decoded PCM frame,
// call enqueue() with interleaved S16 samples.
class QSpkSink : public QObject {
    Q_OBJECT

public:
    QSpkSink(int sample_rate = 48000, int channels = 1);
    ~QSpkSink();

    bool init();

    // Enqueue interleaved S16 samples for playback.
    void enqueue(const int16_t *samples, int num_samples_per_channel);

    void pause();
    void resume();

    bool isValid() const { return audio_sink_ != nullptr && device_ != nullptr; }

private:
    std::unique_ptr<QAudioSink> audio_sink_;
    QIODevice *device_ = nullptr;
    int sample_rate_;
    int channels_;
};


#endif // MEDIA_QSPKSINK_H