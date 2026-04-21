#ifndef PARTICIPANTWIDGET_H
#define PARTICIPANTWIDGET_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLWidget>

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

    std::string& audioTrackSid() { return audio_track_sid_; }
    void setAudioTrackSid(const std::string &audio_track_sid) { audio_track_sid_ = audio_track_sid; }

    std::string& videoTrackSid() { return video_track_sid_; }
    void setVideoTrackSid(const std::string &video_track_sid) { video_track_sid_ = video_track_sid; }

protected:
    void initializeGL() override;
    void paintGL() override;

private:
    void ensureTexture(int width, int height);
    void updateViewportForAspect(int frameWidth, int frameHeight);

private:
    std::string audio_track_sid_;
    std::string video_track_sid_;
    QOpenGLShaderProgram program_;
    QOpenGLTexture *texture_ = nullptr;
};

#endif // PARTICIPANTWIDGET_H
