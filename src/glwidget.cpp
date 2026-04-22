#include "glwidget.h"
#include "media_engine.h"
#include "../tools/avatar_generator.h"

#include <QSurfaceFormat>

GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , audio_track_sid_()
    , video_track_sid_()
{
    QSurfaceFormat fmt = format();
    fmt.setAlphaBufferSize(8);
    setFormat(fmt);
    setAutoFillBackground(false);
    setAttribute(Qt::WA_NoSystemBackground, true);
}

GLWidget::~GLWidget()
{
    makeCurrent();
    delete texture_;
    texture_ = nullptr;
    delete avatar_texture_;
    avatar_texture_ = nullptr;
    doneCurrent();
}

void GLWidget::setAudioTrackSid(const QString &audio_track_sid)
{
    if (audio_track_sid_ == audio_track_sid) {
        return;
    }

    audio_track_sid_ = audio_track_sid;
}

void GLWidget::setParticipantName(const QString &name)
{
    if (participant_name_ == name) {
        return;
    }

    participant_name_ = name;
    if (!name.isEmpty()) {
        const QImage avatar = AvatarGenerator::generateAvatar(name);
        if (!avatar.isNull()) {
            ensureAvatarTexture(avatar);
        }
    }
}

void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glDisable(GL_BLEND);
    glClearColor(0.0f, 0.0f, 0.0f, 0.45f);

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

void GLWidget::paintGL()
{
    VideoFrameBuff tmpBuff{};

    // Force a consistent letterbox/pillarbox tint every frame.
    glClearColor(0.0f, 0.0f, 0.0f, 0.45f);
    glClear(GL_COLOR_BUFFER_BIT);

    int frame_width = 0;
    int frame_height = 0;
    bool has_video = false;

    // Try to get new frame only if SID is not empty
    if (!video_track_sid_.isEmpty()) {
        const bool got_new_frame = MediaEngine::instance().copyVideoFrame(video_track_sid_.toStdString(), tmpBuff);
        if (got_new_frame) {
            frame_width = tmpBuff.width;
            frame_height = tmpBuff.height;
            has_video = true;
            ensureTexture(tmpBuff.width, tmpBuff.height);
            if (texture_) {
                texture_->bind();
                texture_->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, tmpBuff.rgba.data());
                texture_->release();
            }
        }
    }

    // If no video frame, try to use avatar as fallback
    if (!has_video && avatar_texture_) {
        frame_width = avatar_texture_->width();
        frame_height = avatar_texture_->height();
    } else if (texture_ && frame_width <= 0) {
        // Use cached video texture dimensions if no new frame
        frame_width = texture_->width();
        frame_height = texture_->height();
    }

    // Select which texture to render: video > avatar > nothing
    QOpenGLTexture *render_texture = nullptr;
    if (has_video && texture_) {
        render_texture = texture_;
    } else if (!has_video && avatar_texture_) {
        render_texture = avatar_texture_;
    } else if (texture_) {
        render_texture = texture_;
    }

    if (!render_texture || frame_width <= 0 || frame_height <= 0) {
        return;
    }

    updateViewportForAspect(frame_width, frame_height);

    program_.bind();
    render_texture->bind(0);
    program_.setUniformValue("uTex", 0);

    renderTexturedQuad();

    render_texture->release();
    program_.release();
}

void GLWidget::renderTexturedQuad()
{
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

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, uvs);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

void GLWidget::ensureTexture(int width, int height)
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

void GLWidget::ensureAvatarTexture(const QImage &image)
{
    if (image.isNull()) {
        return;
    }

    if (avatar_texture_) {
        delete avatar_texture_;
        avatar_texture_ = nullptr;
    }

    QImage glImage = image.convertToFormat(QImage::Format_RGBA8888);
    if (glImage.isNull()) {
        return;
    }

    makeCurrent();
    avatar_texture_ = new QOpenGLTexture(QOpenGLTexture::Target2D);
    avatar_texture_->setFormat(QOpenGLTexture::RGBA8_UNorm);
    avatar_texture_->setSize(glImage.width(), glImage.height());
    avatar_texture_->setMipLevels(1);
    avatar_texture_->allocateStorage(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8);
    avatar_texture_->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, glImage.constBits());
    avatar_texture_->setMinificationFilter(QOpenGLTexture::Linear);
    avatar_texture_->setMagnificationFilter(QOpenGLTexture::Linear);
    avatar_texture_->setWrapMode(QOpenGLTexture::ClampToEdge);
    doneCurrent();
}

void GLWidget::updateViewportForAspect(int frameWidth, int frameHeight)
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
