#include "participant.h"
#include "ui_participant.h"
#include "glwidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QGraphicsDropShadowEffect>
#include <QResizeEvent>
#include <QVBoxLayout>

#include <algorithm>

Participant::Participant(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Participant)
{
    ui->setupUi(this);

    setObjectName("ParticipantContainer");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(false);

    ui->verticalLayout->setContentsMargins(kBorderWidth, kBorderWidth, kBorderWidth, kBorderWidth);
    ui->verticalLayout->setSpacing(0);

    // 创建 GLWidget 并添加到布局
    glWidget_ = new GLWidget(this);
    ui->verticalLayout->addWidget(glWidget_);

    speakingGlow_ = new QGraphicsDropShadowEffect(this);
    speakingGlow_->setOffset(0, 0);
    speakingGlow_->setBlurRadius(8);
    speakingGlow_->setColor(QColor(39, 201, 63, 230));
    speakingGlow_->setEnabled(false);
    setGraphicsEffect(speakingGlow_);

    setupAudioOverlay();
    updateSpeakingStyle(false);
}

Participant::~Participant()
{
    delete ui;
}

void Participant::setParticipantName(const QString &name)
{
    const QString trimmedName = name.trimmed();
    if (trimmedName.isEmpty() && !participantName_.trimmed().isEmpty()) {
        return;
    }

    if (participantName_ == name) {
        return;
    }

    participantName_ = name;
    if (nameLabel_) {
        nameLabel_->setText(participantName_);
    }
}

void Participant::setAudioTrackSid(const QString &sid)
{
    if (glWidget_) {
        glWidget_->setAudioTrackSid(sid);
    }
}

void Participant::setVideoTrackSid(const QString &sid)
{
    if (glWidget_) {
        glWidget_->setVideoTrackSid(sid);
    }
}

QString Participant::audioTrackSid() const
{
    return glWidget_ ? glWidget_->audioTrackSid() : QString();
}

QString Participant::videoTrackSid() const
{
    return glWidget_ ? glWidget_->videoTrackSid() : QString();
}

void Participant::setAudioStatus(float level, bool speaking)
{
    const int levelInt = static_cast<int>(std::clamp(level * 100.0f, 0.0f, 100.0f));
    if (lastAudioLevel_ == levelInt && lastSpeaking_ == speaking) {
        return;
    }

    lastAudioLevel_ = levelInt;
    lastSpeaking_ = speaking;

    if (levelBar_) {
        levelBar_->setValue(levelInt);
    }

    updateSpeakingStyle(speaking);
}

void Participant::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (audioOverlay_) {
        const int margin = 8;
        const QSize s = audioOverlay_->sizeHint();
        const int w = qMin(s.width(), width() - 2 * margin);
        const int h = s.height();
        audioOverlay_->setGeometry(margin, height() - h - margin, w, h);
        audioOverlay_->raise();
    }
}

void Participant::setupAudioOverlay()
{
    // overlay 挂到 Participant 自身，通过 resizeEvent 定位到左下角
    // 不在 QOpenGLWidget 上添加任何 layout
    audioOverlay_ = new QWidget(this);
    audioOverlay_->setAttribute(Qt::WA_TransparentForMouseEvents);
    audioOverlay_->setStyleSheet(
        "background-color: rgba(10, 14, 20, 145);"
        "border: none;"
        "border-radius: 4px;");

    auto *row = new QHBoxLayout(audioOverlay_);
    row->setContentsMargins(6, 4, 6, 4);
    row->setSpacing(4);

    nameLabel_ = new QLabel("Guest", audioOverlay_);
    nameLabel_->setStyleSheet("color:#E8EDF5; background:transparent; border:none; font-size:14px; font-weight:500;");

    levelBar_ = new QProgressBar(audioOverlay_);
    levelBar_->setRange(0, 100);
    levelBar_->setValue(0);
    levelBar_->setTextVisible(false);
    levelBar_->setOrientation(Qt::Vertical);
    levelBar_->setFixedWidth(6);
    levelBar_->setFixedHeight(nameLabel_->sizeHint().height());
    levelBar_->setStyleSheet(
        "QProgressBar {"
        " border: 1px solid rgba(75, 87, 105, 180);"
        " border-radius: 3px;"
        " background: rgba(8, 10, 14, 155);"
        "}"
        "QProgressBar::chunk {"
        " border-radius: 2px;"
        " background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #27C93F, stop:1 #0B8A2C);"
        "}");
    row->addWidget(levelBar_);
    row->addWidget(nameLabel_, 1);

    audioOverlay_->adjustSize();
}

void Participant::updateSpeakingStyle(bool speaking)
{
    if (speakingGlow_) {
        speakingGlow_->setEnabled(speaking);
    }

    if (speaking) {
        setStyleSheet(
            "#ParticipantContainer {"
            " border: 1px solid #27C93F;"
            " border-radius: 8px;"
            " background-color: #0D1218;"
            "}");
    } else {
        setStyleSheet(
            "#ParticipantContainer {"
            " border: 1px solid #2A3442;"
            " border-radius: 8px;"
            " background-color: #0D1218;"
            "}");
    }
}
