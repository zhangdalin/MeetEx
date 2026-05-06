#include "memberwidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>

#include <algorithm>

namespace {
constexpr const char *kMemberNameNormalStyle =
    "QLabel#memberNameLabel { font-size:14px; font-weight:500; }"
    "QLabel#memberNameLabel:hover { font-weight:700; }";
}

MemberWidget::MemberWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    updateSpeakingStyle(false);
}

void MemberWidget::setName(const QString &name)
{
    const QString trimmedName = name.trimmed();
    if (trimmedName.isEmpty() && !memberName_.trimmed().isEmpty()) {
        return;
    }

    if (memberName_ == name) {
        return;
    }

    memberName_ = name;
    if (nameLabel_) {
        nameLabel_->setText(memberName_);
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

    updateSpeakingStyle(speaking);
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

void MemberWidget::updateSpeakingStyle(bool speaking)
{
    Q_UNUSED(speaking);

    if (!nameLabel_) {
        return;
    }

    nameLabel_->setStyleSheet(kMemberNameNormalStyle);
}