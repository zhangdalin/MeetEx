
#ifndef MEDIA_QCAM_H
#define MEDIA_QCAM_H

#include <QObject>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QVideoFrame>
#include <QVideoSink>

// -------------------------
// QCamSource (Qt-based camera source)
// -------------------------
// Camera source implemented with Qt's native camera API.
// Callback delivers raw RGBA pixels.
// init() starts capture. stop() stops capture. No pump() needed — frames
// are delivered asynchronously via the VideoCallback on the Qt event thread.
class QCamSource : public QObject {
    Q_OBJECT

public:
    // Callback signature used by the media pipeline.
    // pixels : RGBA8888 rows, pitch bytes per row.
    // timestampNs : capture timestamp in nanoseconds.
    using VideoCallback = std::function<void(
        const uint8_t *pixels,
        int pitch,
        int width, int height,
        int64_t timestampNs)>;

    QCamSource(int desired_width = 1280, int desired_height = 720,
               int desired_fps = 30,
               VideoCallback cb = nullptr,
               QObject *parent = nullptr);

    ~QCamSource();

    bool init(); // Open first available camera and start capture
    void stop(); // Stop capture and release camera

    bool isValid() const { return camera_ != nullptr && camera_->isActive(); }

private slots:
    void videoFrameChanged(const QVideoFrame &frame);

private:
    QCamera *camera_ = nullptr;
    QMediaCaptureSession *captureSession_ = nullptr;
    QVideoSink *videoSink_ = nullptr;
    int width_;
    int height_;
    int fps_;
    VideoCallback callback_;
};

#endif // MEDIA_QCAM_H