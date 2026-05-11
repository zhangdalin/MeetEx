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

    const QString& audioTrackSid() const { return audio_track_sid_; }
    void setAudioTrackSid(const QString &audio_track_sid);

    const QString& videoTrackSid() const { return video_track_sid_; }
    void setVideoTrackSid(const QString &video_track_sid);

    const QString& id() const { return participant_id_; }
    void setId(const QString &participant_id);

    const QString& name() const { return participant_name_; }
    void setName(const QString &name);

protected:
    void initializeGL() override;
    void paintGL() override;

private:
    void ensureTexture(int width, int height);
    void ensureAvatarTexture(const QImage &image);
    void updateViewportForAspect(int frameWidth, int frameHeight);
    void renderTexturedQuad(QOpenGLTexture *texture, float alpha = 1.0f);

private:
    QString audio_track_sid_;
    QString video_track_sid_;
    QString participant_id_;
    QString participant_name_;
    QImage avatar_image_;
    bool avatar_texture_dirty_ = false;
    float video_fade_alpha_ = 0.0f;

    QOpenGLShaderProgram program_;
    QOpenGLTexture *texture_ = nullptr;
    QOpenGLTexture *avatar_texture_ = nullptr;
};

#endif // GLWIDGET_H
