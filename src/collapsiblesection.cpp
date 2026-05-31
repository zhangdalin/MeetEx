#include "collapsiblesection.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMouseEvent>

CollapsibleSection::CollapsibleSection(const QString &title, QWidget *parent)
    : QWidget(parent)
    , toggleButton_(nullptr)
    , titleLabel_(nullptr)
    , summaryLabel_(nullptr)
    , contentContainer_(nullptr)
    , contentWidget_(nullptr)
    , expanded_(true)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 标题栏
    QWidget *header = new QWidget(this);
    header->setObjectName("sectionHeader");
    header->setStyleSheet(
        "QWidget#sectionHeader {"
        "  background-color: #f5f5f5;"
        "  border-radius: 6px;"
        "}"
        "QWidget#sectionHeader:hover {"
        "  background-color: #e8e8e8;"
        "}"
    );
    header->setCursor(Qt::PointingHandCursor);

    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(10, 10, 15, 10);
    headerLayout->setSpacing(10);

    toggleButton_ = new QToolButton(this);
    toggleButton_->setStyleSheet("border: none; background: transparent;");
    toggleButton_->setEnabled(false);
    headerLayout->addWidget(toggleButton_);

    titleLabel_ = new QLabel(title, this);
    titleLabel_->setStyleSheet("color: #333333; font-size: 14px; font-weight: bold;");
    headerLayout->addWidget(titleLabel_);
    headerLayout->addStretch();

    summaryLabel_ = new QLabel(this);
    summaryLabel_->setStyleSheet("color: #666666; font-size: 12px;");
    summaryLabel_->setVisible(false);
    headerLayout->addWidget(summaryLabel_);

    mainLayout->addWidget(header);

    contentContainer_ = new QWidget(this);
    contentContainer_->setVisible(true);
    mainLayout->addWidget(contentContainer_);

    QVBoxLayout *contentLayout = new QVBoxLayout(contentContainer_);
    contentLayout->setContentsMargins(10, 10, 10, 10);
    contentLayout->setSpacing(10);

    updateToggleIcon();
}

CollapsibleSection::~CollapsibleSection() = default;

void CollapsibleSection::setContent(QWidget *content)
{
    if (contentWidget_) {
        delete contentWidget_;
    }
    contentWidget_ = content;
    if (contentWidget_) {
        QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(contentContainer_->layout());
        if (layout) {
            layout->addWidget(contentWidget_);
        }
    }
}

QWidget* CollapsibleSection::content() const
{
    return contentWidget_;
}

void CollapsibleSection::setExpanded(bool expanded)
{
    if (expanded_ == expanded) return;
    expanded_ = expanded;
    contentContainer_->setVisible(expanded_);
    summaryLabel_->setVisible(!expanded_ && !summaryLabel_->text().isEmpty());
    updateToggleIcon();
    emit expansionChanged(expanded_);
}

bool CollapsibleSection::isExpanded() const
{
    return expanded_;
}

void CollapsibleSection::setSummary(const QString &summary)
{
    summaryLabel_->setText(summary);
    summaryLabel_->setVisible(!expanded_ && !summary.isEmpty());
}

void CollapsibleSection::onToggleClicked()
{
    setExpanded(!expanded_);
}

void CollapsibleSection::updateToggleIcon()
{
    toggleButton_->setArrowType(expanded_ ? Qt::DownArrow : Qt::RightArrow);
}

void CollapsibleSection::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    onToggleClicked();
}
