#ifndef VIDEOGLWIDGET_H
#define VIDEOGLWIDGET_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLWidget>

#include <cstdint>
#include <vector>
#include <string>
#include <memory>

struct VideoFrameBuff;
class VideoGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit VideoGLWidget(QWidget *parent = nullptr);
    ~VideoGLWidget() override;

    std::string& trackSid() { return track_sid_; }
    void setTrackSid(const std::string &track_sid) { track_sid_ = track_sid; }

protected:
    void initializeGL() override;
    void paintGL() override;

private:
    void ensureTexture(int width, int height);
    void updateViewportForAspect(int frameWidth, int frameHeight);

private:
    std::string track_sid_;

    QOpenGLShaderProgram program_;
    QOpenGLTexture *texture_ = nullptr;
};

#endif // VIDEOGLWIDGET_H
