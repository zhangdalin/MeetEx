#include "participantwidget.h"
#include "meeting_participant.h"
#include "glwidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QGraphicsDropShadowEffect>
#include <QResizeEvent>
#include <QVBoxLayout>

#include <algorithm>

ParticipantWidget::ParticipantWidget(const MeetingParticipant& participant, QWidget *parent)
    : QWidget(parent)
    , isLocalUser_(participant.isLocal())
{
    setupUi();
    setId(participant.id());
    setName(participant.name());
    setAudioTrackSid(participant.audioTrackSid());
    setVideoTrackSid(participant.videoTrackSid());
}

ParticipantWidget::~ParticipantWidget()
{
}

void ParticipantWidget::setupUi()
{
    setObjectName("ParticipantContainer");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(false);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(kBorderWidth, kBorderWidth, kBorderWidth, kBorderWidth);
    mainLayout->setSpacing(0);

    // 创建 GLWidget 并添加到布局
    glWidget_ = new GLWidget(this);
    mainLayout->addWidget(glWidget_);

    speakingGlow_ = new QGraphicsDropShadowEffect(this);
    speakingGlow_->setOffset(0, 0);
    speakingGlow_->setBlurRadius(8);
    speakingGlow_->setColor(QColor(39, 201, 63, 230));
    speakingGlow_->setEnabled(false);
    setGraphicsEffect(speakingGlow_);

    setupAudioOverlay();
    updateSpeakingStyle(false);
}

void ParticipantWidget::setId(const QString &id)
{
    if (glWidget_) {
        glWidget_->setId(id);
    }
}

void ParticipantWidget::setName(const QString &name)
{
    const QString displayName = name.trimmed().isEmpty() ? QStringLiteral("Guest") : name;
    if (nameLabel_) {
        nameLabel_->setText(isLocalUser_ ? QStringLiteral("%1 (我)").arg(displayName) : displayName);
    }
    
    if (glWidget_) {
        glWidget_->setName(displayName);
    }
}

void ParticipantWidget::setAudioTrackSid(const QString &sid)
{
    if (glWidget_) {
        glWidget_->setAudioTrackSid(sid);
    }
}

void ParticipantWidget::setVideoTrackSid(const QString &sid)
{
    if (glWidget_) {
        glWidget_->setVideoTrackSid(sid);
    }
}

QString ParticipantWidget::id() const
{
    return glWidget_ ? glWidget_->id() : QString();
}

QString ParticipantWidget::name() const
{
    return glWidget_ ? glWidget_->name() : QString();
}

QString ParticipantWidget::audioTrackSid() const
{
    return glWidget_ ? glWidget_->audioTrackSid() : QString();
}

QString ParticipantWidget::videoTrackSid() const
{
    return glWidget_ ? glWidget_->videoTrackSid() : QString();
}

void ParticipantWidget::setAudioStatus(float level, bool speaking)
{
    const int levelInt = static_cast<int>(std::clamp(level * 100.0f, 0.0f, 100.0f));
    if (lastAudioLevel_ == levelInt && lastSpeaking_ == speaking) {
        return;
    }

    lastAudioLevel_ = levelInt;
    lastSpeaking_ = speaking;

    updateSpeakingStyle(speaking);
}

void ParticipantWidget::resizeEvent(QResizeEvent *event)
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

void ParticipantWidget::setupAudioOverlay()
{
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
    row->addWidget(nameLabel_, 1);

    audioOverlay_->adjustSize();
}

void ParticipantWidget::updateSpeakingStyle(bool speaking)
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
