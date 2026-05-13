#include "memberwidget.h"
#include "meeting_participant.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>

#include <algorithm>

namespace {
constexpr const char *kMemberNameNormalStyle =
    "QLabel#memberNameLabel { font-size:14px; font-weight:500; }"
    "QLabel#memberNameLabel:hover { font-weight:700; }";
}

MemberWidget::MemberWidget(const QString &memberId, const QString &memberName, bool isLocalUser, QWidget *parent)
    : QWidget(parent)
    , memberId_(memberId)
    , memberName_(memberName)
    , isLocalUser_(isLocalUser)
{
    setupUi();
    setMemberName(memberName_);
}

void MemberWidget::setMemberId(const QString &memberId)
{
    if (memberId_ == memberId) {
        return;
    }

    memberId_ = memberId;
}

void MemberWidget::setMemberName(const QString &memberName)
{
    memberName_ = memberName;
    if (nameLabel_) {
        const QString displayName = memberName_.trimmed().isEmpty() ? QStringLiteral("Guest") : memberName_;
        nameLabel_->setText(isLocalUser_ ? QStringLiteral("%1 (我)").arg(displayName) : displayName);
    }
}

void MemberWidget::setAudioStatus(float level, bool speaking)
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
}

void MemberWidget::setupUi()
{
    setAttribute(Qt::WA_StyledBackground, true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    auto *rowLayout = new QHBoxLayout(this);
    rowLayout->setContentsMargins(4, 4, 4, 4);
    rowLayout->setSpacing(4);

    nameLabel_ = new QLabel("Guest", this);
    nameLabel_->setObjectName(QStringLiteral("memberNameLabel"));
    nameLabel_->setAttribute(Qt::WA_Hover, true);
    nameLabel_->setStyleSheet(kMemberNameNormalStyle);

    levelBar_ = new QProgressBar(this);
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
        " background: transparent;"
        "}"
        "QProgressBar::chunk {"
        " border-radius: 2px;"
        " background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #27C93F, stop:1 #0B8A2C);"
        "}");

    rowLayout->addWidget(nameLabel_, 1);
    rowLayout->addWidget(levelBar_, 0, Qt::AlignRight | Qt::AlignVCenter);
}