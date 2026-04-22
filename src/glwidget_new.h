#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLWidget>
#include <QString>
#include <QImage>

#include <cstdint>
#include <vector>

struct VideoFrameBuff;
class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit GLWidget(QWidget *parent = nullptr);
    ~GLWidget() override;

    QString& audioTrackSid() { return audio_track_sid_; }
    void setAudioTrackSid(const QString &audio_track_sid);

    QString& videoTrackSid() { return video_track_sid_; }
    void setVideoTrackSid(const QString &video_track_sid) { video_track_sid_ = video_track_sid; }

    void setParticipantName(const QString &name);

protected:
    void initializeGL() override;
    void paintGL() override;

private:
    void ensureTexture(int width, int height);
    void ensureAvatarTexture(const QImage &image);
    void updateViewportForAspect(int frameWidth, int frameHeight);
    void renderTexturedQuad();

private:
    QString audio_track_sid_;
    QString video_track_sid_;
    QString participant_name_;

    QOpenGLShaderProgram program_;
    QOpenGLTexture *texture_ = nullptr;
    QOpenGLTexture *avatar_texture_ = nullptr;
};

#endif // GLWIDGET_H
