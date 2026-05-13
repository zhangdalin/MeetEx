
#include "media_qmic.h"

#include <QAudioDevice>
#include <QAudioFormat>
#include <QAudioSink>
#include <QAudioSource>
#include <QIODevice>
#include <QMediaDevices>
#include <QDebug>
#include <QThread>

// ---------------------- QMicSource -----------------------------

QMicSource::QMicSource(int sample_rate, int channels, int frame_samples,
                     AudioCallback cb)
    : sample_rate_(sample_rate), channels_(channels),
    frame_samples_(frame_samples), callback_(std::move(cb)) {}

QMicSource::~QMicSource() {
    audio_source_.reset();
    device_ = nullptr;
}

bool QMicSource::init() {
    const QAudioDevice input_device = QMediaDevices::defaultAudioInput();
    if (input_device.isNull()) {
        qCritical() << QThread::currentThread() << __FUNCTION__ << "No default audio input device";
        return false;
    }

    QAudioFormat format;
    format.setSampleRate(sample_rate_);
    format.setChannelCount(channels_);
    format.setSampleFormat(QAudioFormat::Int16);

    if (!input_device.isFormatSupported(format)) {
        qCritical() << QThread::currentThread() << __FUNCTION__ << "Requested input format is not supported"
                    << format.sampleRate() << format.channelCount() << format.sampleFormat();
        return false;
    }

    audio_source_ = std::make_unique<QAudioSource>(input_device, format);
    device_ = audio_source_->start();
    if (!device_) {
        qCritical() << QThread::currentThread() << __FUNCTION__ << "Failed to start audio input" << audio_source_->error();
        audio_source_.reset();
        return false;
    }

    return true;
}

void QMicSource::pump() {
    if (!device_ || !callback_)
        return;

    const int samples_per_frame_total = frame_samples_ * channels_;
    const qint64 bytes_per_frame = static_cast<qint64>(samples_per_frame_total * sizeof(int16_t));

    const qint64 available = audio_source_->bytesAvailable();
    if (available < bytes_per_frame) {
        return;
    }

    std::vector<int16_t> buffer(samples_per_frame_total);

    const qint64 got_bytes = device_->read(reinterpret_cast<char *>(buffer.data()), bytes_per_frame);

    if (got_bytes <= 0) {
        return;
    }

    const int got_samples_total = static_cast<int>(got_bytes / static_cast<qint64>(sizeof(int16_t)));
    const int got_samples_per_channel = got_samples_total / channels_;

    callback_(buffer.data(), got_samples_per_channel, sample_rate_, channels_);
}

void QMicSource::pause() {
    if (audio_source_) {
        audio_source_->suspend();
    }
}

void QMicSource::resume() {
    if (audio_source_) {
        audio_source_->resume();
    }
}
