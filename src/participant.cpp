#include "participant.h"
#include "ui_participant.h"
#include "participantwidget.h"

#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>

Participant::Participant(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Participant)
{
    ui->setupUi(this);

    // 创建 ParticipantWidget 并添加到布局
    participantWidget_ = new ParticipantWidget(this);
    ui->verticalLayout->addWidget(participantWidget_);
}

Participant::~Participant()
{
    delete ui;
}

void Participant::setParticipantName(const QString &name)
{
    if (participantWidget_) {
        participantWidget_->setParticipantName(name);
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
    if (participantWidget_) {
        participantWidget_->setAudioStatus(level, speaking);
    }
}

void Participant::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    update();
}
