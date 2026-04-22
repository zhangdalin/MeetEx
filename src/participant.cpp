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

    // 设置大小策略为扩展性，使其填满网格单元格
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(false);

    // 留出边框区域，避免视频内容把圆角边框完全盖住
    ui->verticalLayout->setContentsMargins(kBorderWidth, kBorderWidth, kBorderWidth, kBorderWidth);
    ui->verticalLayout->setSpacing(0);

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

void Participant::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);

    const QRectF fillRect = rect().adjusted(1.0, 1.0, -1.0, -1.0);
    painter.setBrush(QColor("#0D1218"));
    painter.drawRoundedRect(fillRect, kCornerRadius, kCornerRadius);

    QPen borderPen(QColor("#2A3442"), kBorderWidth);
    borderPen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(borderPen);
    painter.setBrush(Qt::NoBrush);

    const qreal halfBorder = kBorderWidth / 2.0;
    const QRectF borderRect = rect().adjusted(halfBorder, halfBorder, -halfBorder, -halfBorder);
    painter.drawRoundedRect(borderRect, kCornerRadius, kCornerRadius);
}

void Participant::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    update();
}
