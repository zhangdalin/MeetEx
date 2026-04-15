#ifndef VIDEOGLWIDGET_H
#define VIDEOGLWIDGET_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLWidget>
#include <QString>

#include <cstdint>
#include <vector>

class VideoGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit VideoGLWidget(const QString &participant_identity, const QString &track_sid, QWidget *parent = nullptr);
    ~VideoGLWidget() override;

    const QString& participantIdentity() const { return participant_identity_; }
    const QString& trackSid() const { return track_sid_; }

protected:
    void initializeGL() override;
    void paintGL() override;

private:
    void ensureTexture(int width, int height);
    void updateViewportForAspect(int frameWidth, int frameHeight);

private:
    QString participant_identity_;
    QString track_sid_;
    QOpenGLShaderProgram program_;
    QOpenGLTexture *texture_ = nullptr;
    int frame_width_ = 0;
    int frame_height_ = 0;
    std::vector<std::uint8_t> frame_rgba_;
};

#endif // VIDEOGLWIDGET_H
