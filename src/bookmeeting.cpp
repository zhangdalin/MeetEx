#include "bookmeeting.h"
#include "ui_bookmeeting.h"
#include "collapsiblesection.h"
#include "home.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QMessageBox>
#include <QSettings>
#include <QScreen>
#include <QGuiApplication>
#include <QCloseEvent>
#include <QRegularExpression>

using namespace std;

extern unique_ptr<Home> home;
extern unique_ptr<QWidget> bookmeeting;

MeetingBookingInfo::MeetingBookingInfo()
    : durationMinutes(60), passwordEnabled(false), waitingRoomEnabled(true)
    , joinPermission(0), autoMuteOnEntry(true), screenSharePermission(0)
    , recordingPermission(0), meetingNumberType(0) {}

BookMeeting::BookMeeting(QWidget *parent)
    : QWidget(parent), ui(new Ui::BookMeeting)
    , hasUnsavedChanges_(false)
{
    ui->setupUi(this);
    setupUI();
    setupCollapsibleSections();
    setupConnections();
    loadOptions();
    loadDraft();
}

BookMeeting::~BookMeeting() { delete ui; }

void BookMeeting::setupUI()
{
    setWindowTitle(tr("预定会议"));

    setStyleSheet(R"(
        QLabel#basicGroupLabel, QLabel#attendeesGroupLabel {
            background-color: #f5f5f5;
            color: #666666;
            padding: 5px 15px;
            border-radius: 3px;
            font-size: 10pt;
            font-weight: bold;
        }
        QLineEdit, QComboBox, QDateEdit, QTimeEdit, QTextEdit {
            border: 1px solid #e0e0e0;
            border-radius: 4px;
            padding: 6px 10px;
            font-size: 13px;
            background-color: #ffffff;
            min-height: 32px;
        }
        QLineEdit:focus, QComboBox:focus, QTextEdit:focus,
        QDateEdit:focus, QTimeEdit:focus {
            border-color: #0078d4;
            border-width: 1.5px;
        }
        QLineEdit::placeholder {
            color: #999999;
        }
        QPushButton#bookNowBtn {
            background-color: #0078d4;
            border: none;
            border-radius: 6px;
            padding: 8px 20px;
            color: white;
            font-size: 13px;
            font-weight: bold;
            min-height: 36px;
            min-width: 80px;
        }
        QPushButton#bookNowBtn:hover {
            background-color: #006cbd;
        }
        QPushButton#saveDraftBtn {
            background-color: #ffffff;
            border: 1px solid #0078d4;
            border-radius: 6px;
            padding: 8px 20px;
            color: #0078d4;
            font-size: 13px;
            min-height: 36px;
            min-width: 80px;
        }
        QPushButton#saveDraftBtn:hover {
            background-color: #f0f7ff;
        }
        QPushButton#cancelBtn {
            background-color: #ffffff;
            border: 1px solid #e0e0e0;
            border-radius: 6px;
            padding: 8px 20px;
            color: #333333;
            font-size: 13px;
            min-height: 36px;
            min-width: 80px;
        }
        QPushButton#cancelBtn:hover {
            background-color: #f5f5f5;
        }
    )");

    ui->bookNowBtn->setObjectName("bookNowBtn");
    ui->saveDraftBtn->setObjectName("saveDraftBtn");
    ui->cancelBtn->setObjectName("cancelBtn");

    if (QScreen *screen = QGuiApplication::primaryScreen()) {
        QRect g = screen->availableGeometry();
        move((g.width() - 460) / 2, (g.height() - 620) / 2);
    }
    resize(460, 620);

    ui->topicEdit->addAction(QIcon(":/assets/user.png"), QLineEdit::LeadingPosition);
    ui->emailsEdit->addAction(QIcon(":/assets/email.png"), QLineEdit::LeadingPosition);

    ui->dateEdit->setDate(QDate::currentDate());
    ui->timeEdit->setTime(QTime::currentTime().addSecs(1800));
    ui->dateEdit->setMinimumDate(QDate::currentDate());
}

void BookMeeting::setupCollapsibleSections()
{
    QVBoxLayout *collapsibleLayout = qobject_cast<QVBoxLayout*>(ui->collapsibleContainer->layout());
    if (!collapsibleLayout) return;

    securitySection_ = new CollapsibleSection(tr("会议安全"), this);
    securitySection_->setExpanded(false);

    QWidget *securityContent = new QWidget();
    QFormLayout *securityForm = new QFormLayout(securityContent);
    securityForm->setSpacing(12);
    securityForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QCheckBox *passwordCheck = new QCheckBox(tr("启用会议密码"));
    passwordCheck->setObjectName("passwordCheck");
    securityForm->addRow(passwordCheck);

    QLineEdit *passwordEdit = new QLineEdit();
    passwordEdit->setObjectName("passwordEdit");
    passwordEdit->setPlaceholderText(tr("4-10位数字"));
    passwordEdit->setEnabled(false);
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->addAction(QIcon(":/assets/lock.png"), QLineEdit::LeadingPosition);
    securityForm->addRow(tr("密码:"), passwordEdit);

    connect(passwordCheck, &QCheckBox::toggled, passwordEdit, &QLineEdit::setEnabled);

    QCheckBox *waitingRoomCheck = new QCheckBox(tr("启用等候室"));
    waitingRoomCheck->setObjectName("waitingRoomCheck");
    waitingRoomCheck->setChecked(true);
    securityForm->addRow(waitingRoomCheck);

    QComboBox *permissionCombo = new QComboBox();
    permissionCombo->setObjectName("permissionCombo");
    permissionCombo->addItem(tr("所有人"), 0);
    permissionCombo->addItem(tr("登录用户"), 1);
    permissionCombo->addItem(tr("仅邀请者"), 2);
    securityForm->addRow(tr("入会权限:"), permissionCombo);

    securitySection_->setContent(securityContent);
    collapsibleLayout->addWidget(securitySection_);

    settingsSection_ = new CollapsibleSection(tr("会议设置"), this);
    settingsSection_->setExpanded(false);

    QWidget *settingsContent = new QWidget();
    QFormLayout *settingsForm = new QFormLayout(settingsContent);
    settingsForm->setSpacing(12);
    settingsForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QCheckBox *muteCheck = new QCheckBox(tr("参会者入会自动静音"));
    muteCheck->setObjectName("muteCheck");
    muteCheck->setChecked(true);
    settingsForm->addRow(muteCheck);

    QComboBox *shareCombo = new QComboBox();
    shareCombo->setObjectName("shareCombo");
    shareCombo->addItem(tr("所有人"), 0);
    shareCombo->addItem(tr("仅主持人"), 1);
    settingsForm->addRow(tr("屏幕共享权限:"), shareCombo);

    QComboBox *recordCombo = new QComboBox();
    recordCombo->setObjectName("recordCombo");
    recordCombo->addItem(tr("仅主持人"), 0);
    recordCombo->addItem(tr("所有人"), 1);
    settingsForm->addRow(tr("允许录制:"), recordCombo);

    settingsSection_->setContent(settingsContent);
    collapsibleLayout->addWidget(settingsSection_);

    advancedSection_ = new CollapsibleSection(tr("高级选项"), this);
    advancedSection_->setExpanded(false);

    QWidget *advancedContent = new QWidget();
    QFormLayout *advancedForm = new QFormLayout(advancedContent);
    advancedForm->setSpacing(12);
    advancedForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QComboBox *numberTypeCombo = new QComboBox();
    numberTypeCombo->setObjectName("numberTypeCombo");
    numberTypeCombo->addItem(tr("自动生成"), 0);
    numberTypeCombo->addItem(tr("使用个人会议号"), 1);
    advancedForm->addRow(tr("会议号:"), numberTypeCombo);

    QTextEdit *descEdit = new QTextEdit();
    descEdit->setObjectName("descEdit");
    descEdit->setPlaceholderText(tr("添加会议描述（可选）"));
    descEdit->setMaximumHeight(80);
    advancedForm->addRow(tr("描述:"), descEdit);

    advancedSection_->setContent(advancedContent);
    collapsibleLayout->addWidget(advancedSection_);
}

void BookMeeting::setupConnections()
{
    connect(ui->cancelBtn, &QPushButton::clicked, this, &BookMeeting::onCancelClicked);
    connect(ui->saveDraftBtn, &QPushButton::clicked, this, &BookMeeting::onSaveDraftClicked);
    connect(ui->bookNowBtn, &QPushButton::clicked, this, &BookMeeting::onBookNowClicked);

    connect(ui->topicEdit, &QLineEdit::textChanged, this, [this]() { hasUnsavedChanges_ = true; });
    connect(ui->emailsEdit, &QLineEdit::textChanged, this, [this]() { hasUnsavedChanges_ = true; });
}

void BookMeeting::updateSectionSummaries()
{
    // TODO: 实现更新各分组摘要信息的功能
}

void BookMeeting::loadOptions()
{
    ui->durationCombo->addItem(tr("15 分钟"), 15);
    ui->durationCombo->addItem(tr("30 分钟"), 30);
    ui->durationCombo->addItem(tr("45 分钟"), 45);
    ui->durationCombo->addItem(tr("60 分钟"), 60);
    ui->durationCombo->addItem(tr("90 分钟"), 90);
    ui->durationCombo->addItem(tr("120 分钟"), 120);
    ui->durationCombo->setCurrentIndex(3);

    ui->timezoneCombo->addItem(tr("(UTC+08:00) 北京"), "Asia/Shanghai");
    ui->timezoneCombo->addItem(tr("(UTC+09:00) 东京"), "Asia/Tokyo");
    ui->timezoneCombo->addItem(tr("(UTC+00:00) 伦敦"), "Europe/London");
    ui->timezoneCombo->addItem(tr("(UTC-05:00) 纽约"), "America/New_York");

    ui->roomCombo->addItem(tr("不选择会议室"), "");
    ui->roomCombo->addItem(tr("会议室 A"), "room_a");
    ui->roomCombo->addItem(tr("会议室 B"), "room_b");
}

void BookMeeting::closeEvent(QCloseEvent *event)
{
    if (hasUnsavedChanges_) {
        auto reply = QMessageBox::question(this, tr("未保存的更改"),
            tr("有未保存的更改，是否保存草稿？"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Cancel);
        if (reply == QMessageBox::Save) {
            saveDraft();
        } else if (reply == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
    }
    emit sigClosing();
    QWidget::closeEvent(event);
}

void BookMeeting::onCancelClicked()
{
    emit sigCancelled();
    close();
}

void BookMeeting::onSaveDraftClicked()
{
    saveDraft();
    emit sigDraftSaved();
    hasUnsavedChanges_ = false;
    QMessageBox::information(this, tr("保存成功"), tr("草稿已保存"));
}

void BookMeeting::onBookNowClicked()
{
    if (!validateForm()) return;
    MeetingBookingInfo info = getBookingInfo();
    QSettings settings("MeetEx", "BookMeeting");
    settings.remove("draft");
    hasUnsavedChanges_ = false;
    emit sigMeetingBooked(info);
    QMessageBox::information(this, tr("预定成功"), tr("会议 \"%1\" 已成功预定").arg(info.topic));
    close();
}

bool BookMeeting::validateForm()
{
    if (ui->topicEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("验证失败"), tr("请输入会议主题"));
        ui->topicEdit->setFocus();
        return false;
    }

    QDateTime selected(ui->dateEdit->date(), ui->timeEdit->time());
    if (selected < QDateTime::currentDateTime()) {
        QMessageBox::warning(this, tr("验证失败"), tr("开始时间必须晚于当前时间"));
        return false;
    }

    return true;
}

MeetingBookingInfo BookMeeting::getBookingInfo() const
{
    MeetingBookingInfo info;

    info.topic = ui->topicEdit->text();
    info.startTime = QDateTime(ui->dateEdit->date(), ui->timeEdit->time());
    info.durationMinutes = ui->durationCombo->currentData().toInt();
    info.timeZone = ui->timezoneCombo->currentData().toString();

    info.inviteEmails = ui->emailsEdit->text().split(QRegularExpression("[,;]"), Qt::SkipEmptyParts);
    info.roomResource = ui->roomCombo->currentData().toString();

    if (QCheckBox *passwordCheck = securitySection_->content()->findChild<QCheckBox*>("passwordCheck"))
        info.passwordEnabled = passwordCheck->isChecked();
    if (QLineEdit *passwordEdit = securitySection_->content()->findChild<QLineEdit*>("passwordEdit"))
        info.password = passwordEdit->text();
    if (QCheckBox *waitingRoomCheck = securitySection_->content()->findChild<QCheckBox*>("waitingRoomCheck"))
        info.waitingRoomEnabled = waitingRoomCheck->isChecked();
    if (QComboBox *permissionCombo = securitySection_->content()->findChild<QComboBox*>("permissionCombo"))
        info.joinPermission = permissionCombo->currentData().toInt();

    if (QCheckBox *muteCheck = settingsSection_->content()->findChild<QCheckBox*>("muteCheck"))
        info.autoMuteOnEntry = muteCheck->isChecked();
    if (QComboBox *shareCombo = settingsSection_->content()->findChild<QComboBox*>("shareCombo"))
        info.screenSharePermission = shareCombo->currentData().toInt();
    if (QComboBox *recordCombo = settingsSection_->content()->findChild<QComboBox*>("recordCombo"))
        info.recordingPermission = recordCombo->currentData().toInt();

    if (QComboBox *numberTypeCombo = advancedSection_->content()->findChild<QComboBox*>("numberTypeCombo"))
        info.meetingNumberType = numberTypeCombo->currentData().toInt();
    if (QTextEdit *descEdit = advancedSection_->content()->findChild<QTextEdit*>("descEdit"))
        info.description = descEdit->toPlainText();

    return info;
}

void BookMeeting::saveDraft()
{
    MeetingBookingInfo info = getBookingInfo();
    QSettings settings("MeetEx", "BookMeeting");
    settings.setValue("draft/topic", info.topic);
    settings.setValue("draft/startTime", info.startTime);
    settings.setValue("draft/duration", info.durationMinutes);
    settings.setValue("draft/timeZone", info.timeZone);
    settings.setValue("draft/emails", info.inviteEmails.join(", "));
    settings.setValue("draft/hasData", true);
}

void BookMeeting::loadDraft()
{
    QSettings settings("MeetEx", "BookMeeting");
    if (!settings.value("draft/hasData", false).toBool()) return;

    ui->topicEdit->setText(settings.value("draft/topic").toString());
    ui->dateEdit->setDate(settings.value("draft/startTime").toDateTime().date());
    ui->timeEdit->setTime(settings.value("draft/startTime").toDateTime().time());

    int duration = settings.value("draft/duration", 60).toInt();
    int index = ui->durationCombo->findData(duration);
    if (index >= 0) ui->durationCombo->setCurrentIndex(index);

    ui->emailsEdit->setText(settings.value("draft/emails").toString());
}

bool BookMeeting::hasDraft() const
{
    QSettings settings("MeetEx", "BookMeeting");
    return settings.value("draft/hasData", false).toBool();
}

void BookMeeting::setBookingInfo(const MeetingBookingInfo &info)
{
    ui->topicEdit->setText(info.topic);
    ui->dateEdit->setDate(info.startTime.date());
    ui->timeEdit->setTime(info.startTime.time());

    int index = ui->durationCombo->findData(info.durationMinutes);
    if (index >= 0) ui->durationCombo->setCurrentIndex(index);
}
