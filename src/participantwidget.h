#ifndef PARTICIPANTWIDGET_H
#define PARTICIPANTWIDGET_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLWidget>
#include <QPointer>
#include <QString>

class QLabel;
class QProgressBar;
class QWidget;

#include <cstdint>
#include <vector>
#include <string>
#include <memory>

struct VideoFrameBuff;
class ParticipantWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit ParticipantWidget(QWidget *parent = nullptr);
    ~ParticipantWidget() override;

    QString& audioTrackSid() { return audio_track_sid_; }
    void setAudioTrackSid(const QString &audio_track_sid);

    QString& videoTrackSid() { return video_track_sid_; }
    void setVideoTrackSid(const QString &video_track_sid) { video_track_sid_ = video_track_sid; }

    void setParticipantName(const QString &name);
    void setAudioStatus(float level, bool speaking);

protected:
    void initializeGL() override;
    void paintGL() override;

private:
    void ensureTexture(int width, int height);
    void updateViewportForAspect(int frameWidth, int frameHeight);
    void updateSpeakingStyle(bool speaking);

private:
    void setupOverlay();

private:
    QString audio_track_sid_;
    QString video_track_sid_;

    QOpenGLShaderProgram program_;
    QOpenGLTexture *texture_ = nullptr;

    QPointer<QWidget> audioOverlay_;
    QPointer<QLabel> nameLabel_;
    QPointer<QLabel> stateLabel_;
    QPointer<QProgressBar> levelBar_;

    QString participantName_;
    int lastAudioLevel_ = -1;
    bool lastSpeaking_ = false;
};

#endif // PARTICIPANTWIDGET_H
