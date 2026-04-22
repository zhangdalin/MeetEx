#include "participant.h"
#include "ui_participant.h"
#include "participantwidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
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

    // 创建 ParticipantWidget 并添加到布局
    participantWidget_ = new ParticipantWidget(this);
    ui->verticalLayout->addWidget(participantWidget_);

    setupAudioOverlay();
    updateSpeakingStyle(false);
}

Participant::~Participant()
{
    delete ui;
}

void Participant::setParticipantName(const QString &name)
{
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
    if (participantWidget_) {
        participantWidget_->setAudioTrackSid(sid);
    }
}

void Participant::setVideoTrackSid(const QString &sid)
{
    if (participantWidget_) {
        participantWidget_->setVideoTrackSid(sid);
    }
}

QString Participant::audioTrackSid() const
{
    return participantWidget_ ? participantWidget_->audioTrackSid() : QString();
}

QString Participant::videoTrackSid() const
{
    return participantWidget_ ? participantWidget_->videoTrackSid() : QString();
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
        audioOverlay_->raise();
    }
}

void Participant::setupAudioOverlay()
{
    if (!participantWidget_) {
        return;
    }

    audioOverlay_ = new QWidget(participantWidget_);
    audioOverlay_->setStyleSheet(
        "background-color: rgba(10, 14, 20, 145);"
        "border: 1px solid rgba(100, 115, 135, 130);"
        "border-radius: 6px;");

    auto *rootLayout = new QVBoxLayout(participantWidget_);
    rootLayout->setContentsMargins(8, 8, 8, 8);
    rootLayout->addStretch();
    rootLayout->addWidget(audioOverlay_);

    auto *overlayLayout = new QVBoxLayout(audioOverlay_);
    overlayLayout->setContentsMargins(8, 6, 8, 6);
    overlayLayout->setSpacing(4);

    auto *topRow = new QHBoxLayout();
    topRow->setSpacing(6);
    overlayLayout->addLayout(topRow);

    nameLabel_ = new QLabel("参与者", audioOverlay_);
    nameLabel_->setStyleSheet("color:#E8EDF5;");
    topRow->addWidget(nameLabel_, 1);

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
    topRow->addWidget(levelBar_);
}

void Participant::updateSpeakingStyle(bool speaking)
{
    if (speaking) {
        setStyleSheet(
            "#ParticipantContainer {"
            " border: 2px solid #27C93F;"
            " border-radius: 6px;"
            " background-color: #0D1218;"
            "}");
    } else {
        setStyleSheet(
            "#ParticipantContainer {"
            " border: 1px solid #2A3442;"
            " border-radius: 6px;"
            " background-color: #0D1218;"
            "}");
    }
}
