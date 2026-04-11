#ifndef GLVIDEOWIDGET_H
#define GLVIDEOWIDGET_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLWidget>

#include <cstdint>
#include <vector>

class GLVideoWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit GLVideoWidget(QWidget *parent = nullptr);
    ~GLVideoWidget() override;

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

private:
    void ensureTexture(int width, int height);
    void updateViewportForAspect(int frameWidth, int frameHeight);

    QOpenGLShaderProgram program_;
    QOpenGLTexture *texture_ = nullptr;
    int frame_width_ = 0;
    int frame_height_ = 0;
    std::vector<std::uint8_t> frame_rgba_;
};

#endif // GLVIDEOWIDGET_H
