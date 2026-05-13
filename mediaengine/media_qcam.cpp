#include "media_qcam.h"

#include <QCameraDevice>
#include <QMediaDevices>
#include <QDebug>
#include <QThread>

QCamSource::QCamSource(int desired_width, int desired_height, int desired_fps,
                       VideoCallback cb, QObject *parent)
    : QObject(parent)
    , width_(desired_width)
    , height_(desired_height)
    , fps_(desired_fps)
    , callback_(std::move(cb))
{
}

QCamSource::~QCamSource()
{
    stop();
}

bool QCamSource::init()
{
    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    if (cameras.isEmpty()) {
        qWarning() << QThread::currentThread() << __FUNCTION__ << "No camera devices found";
        return false;
    }

    // Pick the first available camera
    const QCameraDevice &device = cameras.first();
    qInfo() << QThread::currentThread() << __FUNCTION__ << "Opening camera:" << device.description();

    camera_ = new QCamera(device, this);
    captureSession_ = new QMediaCaptureSession(this);
    videoSink_ = new QVideoSink(this);

    captureSession_->setCamera(camera_);
    captureSession_->setVideoSink(videoSink_);

    connect(videoSink_, &QVideoSink::videoFrameChanged,
            this, &QCamSource::videoFrameChanged);

    camera_->start();

    if (!camera_->isActive()) {
        qCritical() << QThread::currentThread() << __FUNCTION__ << "Failed to start camera";
        stop();
        return false;
    }

    qInfo() << QThread::currentThread() << __FUNCTION__ << "Camera started:" << device.description();
    return true;
}

void QCamSource::stop()
{
    if (camera_) {
        camera_->stop();
    }

    // Qt parent-child ownership: deleting captureSession_ first avoids
    // dangling references from the session to camera_/videoSink_.
    delete captureSession_;
    captureSession_ = nullptr;

    delete videoSink_;
    videoSink_ = nullptr;

    delete camera_;
    camera_ = nullptr;
}

void QCamSource::videoFrameChanged(const QVideoFrame &frame)
{
    if (!callback_ || !frame.isValid()) {
        return;
    }

    // Use QVideoFrame::toImage() for reliable pixel access across formats
    const QImage img = frame.toImage().convertToFormat(QImage::Format_RGBA8888);
    if (img.isNull()) {
        qWarning() << QThread::currentThread() << __FUNCTION__ << "Failed to convert frame to RGBA8888";
        return;
    }

    const int64_t timestampNs = frame.startTime() * 1000; // microseconds -> nanoseconds

    callback_(img.constBits(),
              img.bytesPerLine(),
              img.width(),
              img.height(),
              timestampNs);
}
