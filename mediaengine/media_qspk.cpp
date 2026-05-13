#include "media_qspk.h"

#include <QAudioDevice>
#include <QAudioFormat>
#include <QMediaDevices>
#include <QDebug>

#include <chrono>
#include <thread>
#include <QThread>

QSpkSink::QSpkSink(int sample_rate, int channels)
    : sample_rate_(sample_rate), channels_(channels) {}

QSpkSink::~QSpkSink() {
    audio_sink_.reset();
    device_ = nullptr;
}

bool QSpkSink::init() {
    const QAudioDevice output_device = QMediaDevices::defaultAudioOutput();
    if (output_device.isNull()) {
        qCritical() << QThread::currentThread() << __FUNCTION__ << "No default audio output device";
        return false;
    }

    QAudioFormat format;
    format.setSampleRate(sample_rate_);
    format.setChannelCount(channels_);
    format.setSampleFormat(QAudioFormat::Int16);

    if (!output_device.isFormatSupported(format)) {
        qCritical() << QThread::currentThread() << __FUNCTION__ 
                    << "Requested output format is not supported"
                    << format.sampleRate() << format.channelCount() << format.sampleFormat();
        return false;
    }

    audio_sink_ = std::make_unique<QAudioSink>(output_device, format);
    device_ = audio_sink_->start();
    if (!device_) {
        qCritical() << QThread::currentThread() << __FUNCTION__ << "Failed to start audio output" << audio_sink_->error();
        audio_sink_.reset();
        return false;
    }

    return true;
}

void QSpkSink::enqueue(const int16_t *samples,
                          int num_samples_per_channel) {
    if (!device_ || !samples)
        return;

    const int totalSamples = num_samples_per_channel * channels_;
    const char *data = reinterpret_cast<const char *>(samples);
    qint64 remaining = static_cast<qint64>(totalSamples * static_cast<int>(sizeof(int16_t)));

    static constexpr int kMaxRetries = 20; // 20 x 1ms = up to 20ms total wait
    int retries = 0;
    while (remaining > 0) {
        const qint64 written = device_->write(data, remaining);
        if (written <= 0) {
            if (++retries > kMaxRetries) {
                qWarning() << QThread::currentThread() << __FUNCTION__ << "Audio sink write stalled, dropping" << remaining << "bytes";
                break;
            }
            // Sink buffer full; wait 1ms for hardware to drain then retry
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        retries = 0;
        data += written;
        remaining -= written;
    }
}

void QSpkSink::pause() {
    if (audio_sink_) {
        audio_sink_->suspend();
    }
}

void QSpkSink::resume() {
    if (audio_sink_) {
        audio_sink_->resume();
    }
}
