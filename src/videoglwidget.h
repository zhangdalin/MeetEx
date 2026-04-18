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

    std::string& participantIdentity() { return participant_identity_; }
    std::string& trackSid() { return track_sid_; }
    bool isLocal() { return is_local_; }

    void setParticipantIdentity(const std::string &participant_identity) { participant_identity_ = participant_identity; }
    void setTrackSid(const std::string &track_sid) { track_sid_ = track_sid; }
    void setLocal(bool is_local = true) { is_local_ = is_local; }

protected:
    void initializeGL() override;
    void paintGL() override;

private:
    void ensureTexture(int width, int height);
    void updateViewportForAspect(int frameWidth, int frameHeight);

private:
    std::string participant_identity_;
    std::string track_sid_;
    bool is_local_ = false;

    QOpenGLShaderProgram program_;
    QOpenGLTexture *texture_ = nullptr;
};

#endif // VIDEOGLWIDGET_H
