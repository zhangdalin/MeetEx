#include "participantwidget.h"
#include "media_engine.h"

ParticipantWidget::ParticipantWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , audio_track_sid_()
    , video_track_sid_()
{
}

ParticipantWidget::~ParticipantWidget()
{
    makeCurrent();
    delete texture_;
    texture_ = nullptr;
    doneCurrent();
}

void ParticipantWidget::setAudioTrackSid(const QString &audio_track_sid)
{
    if (audio_track_sid_ == audio_track_sid) {
        return;
    }

    audio_track_sid_ = audio_track_sid;
}

void ParticipantWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

    const char *vertex_shader =
        "attribute vec2 aPos;"
        "attribute vec2 aUv;"
        "varying vec2 vUv;"
        "void main() {"
        "  vUv = aUv;"
        "  gl_Position = vec4(aPos, 0.0, 1.0);"
        "}";

    const char *fragment_shader =
        "varying vec2 vUv;"
        "uniform sampler2D uTex;"
        "void main() {"
        "  gl_FragColor = texture2D(uTex, vUv);"
        "}";

    program_.addShaderFromSourceCode(QOpenGLShader::Vertex, vertex_shader);
    program_.addShaderFromSourceCode(QOpenGLShader::Fragment, fragment_shader);
    program_.bindAttributeLocation("aPos", 0);
    program_.bindAttributeLocation("aUv", 1);
    program_.link();
}

void ParticipantWidget::paintGL()
{
    VideoFrameBuff tmpBuff{};

    if (video_track_sid_.isEmpty()) {
        glClear(GL_COLOR_BUFFER_BIT);
    
            qDebug() << "ParticipantWidget::paintGL, size=" << width() << "x" << height() 
                     << "video_track_sid=" << video_track_sid_;
        return;
    }

    if (MediaEngine::instance().copyVideoFrame(video_track_sid_.toStdString(), tmpBuff)) {
        ensureTexture(tmpBuff.width, tmpBuff.height);
        if (texture_) {
            texture_->bind();
            texture_->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, tmpBuff.rgba.data());
            texture_->release();
        }
    }

    glClear(GL_COLOR_BUFFER_BIT);

    if (!texture_ || tmpBuff.width <= 0 || tmpBuff.height <= 0) {
        return;
    }

    updateViewportForAspect(tmpBuff.width, tmpBuff.height);

    static const GLfloat vertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f,
    };

    static const GLfloat uvs[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
    };

    program_.bind();
    texture_->bind(0);
    program_.setUniformValue("uTex", 0);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, uvs);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    texture_->release();
    program_.release();
}

void ParticipantWidget::ensureTexture(int width, int height)
{
    if (width <= 0 || height <= 0) {
        return;
    }

    if (texture_ && (texture_->width() != width || texture_->height() != height)) {
        delete texture_;
        texture_ = nullptr;
    }

    if (!texture_) {
        texture_ = new QOpenGLTexture(QOpenGLTexture::Target2D);
        texture_->setFormat(QOpenGLTexture::RGBA8_UNorm);
        texture_->setSize(width, height);
        texture_->setMipLevels(1);
        texture_->allocateStorage(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8);
        texture_->setMinificationFilter(QOpenGLTexture::Linear);
        texture_->setMagnificationFilter(QOpenGLTexture::Linear);
        texture_->setWrapMode(QOpenGLTexture::ClampToEdge);
    }
}

void ParticipantWidget::updateViewportForAspect(int frameWidth, int frameHeight)
{
    const qreal dpr = devicePixelRatioF();
    const int view_w = static_cast<int>(width() * dpr);
    const int view_h = static_cast<int>(height() * dpr);
    if (view_w <= 0 || view_h <= 0 || frameWidth <= 0 || frameHeight <= 0) {
        glViewport(0, 0, view_w, view_h);
        return;
    }

    const float frame_aspect = static_cast<float>(frameWidth) / static_cast<float>(frameHeight);
    const float view_aspect = static_cast<float>(view_w) / static_cast<float>(view_h);

    int vp_x = 0;
    int vp_y = 0;
    int vp_w = view_w;
    int vp_h = view_h;

    if (view_aspect > frame_aspect) {
        vp_w = static_cast<int>(view_h * frame_aspect);
        vp_x = (view_w - vp_w) / 2;
    } else {
        vp_h = static_cast<int>(view_w / frame_aspect);
        vp_y = (view_h - vp_h) / 2;
    }

    glViewport(vp_x, vp_y, vp_w, vp_h);
}
