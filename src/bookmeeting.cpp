#include "bookmeeting.h"
#include "ui_bookmeeting.h"
#include "collapsiblesection.h"
#include "home.h"

#include <QScrollArea>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QLabel>
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
    , recordingPermission(0), meetingNumberType(0), addToOutlook(false)
    , addToGoogle(false), addToSystemCalendar(false) {}

BookMeeting::BookMeeting(QWidget *parent)
    : QWidget(parent), ui(new Ui::BookMeeting), basicInfoSection_(nullptr)
    , attendeesSection_(nullptr), securitySection_(nullptr), settingsSection_(nullptr)
    , advancedSection_(nullptr), hasUnsavedChanges_(false)
{
    ui->setupUi(this);
    setupUI();
    setupSections();
    setupConnections();
    loadDraft();
}

BookMeeting::~BookMeeting() { delete ui; }

void BookMeeting::setupUI()
{
    setWindowTitle(tr("预定会议"));
    setMinimumSize(500, 400);
    resize(600, 500);
    if (QScreen *screen = QGuiApplication::primaryScreen()) {
        QRect g = screen->availableGeometry();
        move((g.width() - width()) / 2, (g.height() - height()) / 2);
    }
}

void BookMeeting::setupSections()
{
    QVBoxLayout *scrollLayout = qobject_cast<QVBoxLayout*>(ui->scrollContent->layout());
    if (!scrollLayout) return;

    basicInfoSection_ = new CollapsibleSection(tr("基本信息"), this);
    basicInfoSection_->setExpanded(true);
    QWidget *basicContent = new QWidget();
    QFormLayout *basicForm = new QFormLayout(basicContent);
    basicForm->setSpacing(12);

    QLineEdit *topicEdit = new QLineEdit();
    topicEdit->setObjectName("topicEdit");
    topicEdit->setPlaceholderText(tr("请输入会议主题"));
    basicForm->addRow(tr("会议主题 *"), topicEdit);

    QHBoxLayout *timeLayout = new QHBoxLayout();
    QDateEdit *dateEdit = new QDateEdit(QDate::currentDate());
    dateEdit->setObjectName("dateEdit");
    dateEdit->setCalendarPopup(true);
    dateEdit->setMinimumDate(QDate::currentDate());
    timeLayout->addWidget(dateEdit);
    QTimeEdit *timeEdit = new QTimeEdit(QTime::currentTime().addSecs(1800));
    timeEdit->setObjectName("timeEdit");
    timeEdit->setDisplayFormat("HH:mm");
    timeLayout->addWidget(timeEdit);
    timeLayout->addStretch();
    basicForm->addRow(tr("开始时间 *"), timeLayout);

    QComboBox *durationCombo = new QComboBox();
    durationCombo->setObjectName("durationCombo");
    durationCombo->addItem(tr("15 分钟"), 15);
    durationCombo->addItem(tr("30 分钟"), 30);
    durationCombo->addItem(tr("45 分钟"), 45);
    durationCombo->addItem(tr("60 分钟"), 60);
    durationCombo->addItem(tr("90 分钟"), 90);
    durationCombo->addItem(tr("120 分钟"), 120);
    durationCombo->setCurrentIndex(3);
    basicForm->addRow(tr("持续时间 *"), durationCombo);

    QComboBox *tzCombo = new QComboBox();
    tzCombo->setObjectName("timezoneCombo");
    tzCombo->addItem(tr("(UTC+08:00) 北京"), "Asia/Shanghai");
    tzCombo->addItem(tr("(UTC+09:00) 东京"), "Asia/Tokyo");
    tzCombo->addItem(tr("(UTC+00:00) 伦敦"), "Europe/London");
    tzCombo->addItem(tr("(UTC-05:00) 纽约"), "America/New_York");
    basicForm->addRow(tr("时区"), tzCombo);

    basicInfoSection_->setContent(basicContent);
    scrollLayout->addWidget(basicInfoSection_);

    connect(topicEdit, &QLineEdit::textChanged, this, [this]() {
        hasUnsavedChanges_ = true;
        updateSectionSummaries();
    });

    attendeesSection_ = new CollapsibleSection(tr("参会人员"), this);
    attendeesSection_->setExpanded(true);
    QWidget *attendeesContent = new QWidget();
    QFormLayout *attendeesForm = new QFormLayout(attendeesContent);
    attendeesForm->setSpacing(12);

    QLineEdit *emailsEdit = new QLineEdit();
    emailsEdit->setObjectName("emailsEdit");
    emailsEdit->setPlaceholderText(tr("输入邮箱，用逗号或分号分隔"));
    attendeesForm->addRow(tr("邀请"), emailsEdit);

    QComboBox *roomCombo = new QComboBox();
    roomCombo->setObjectName("roomCombo");
    roomCombo->addItem(tr("不选择会议室"), "");
    roomCombo->addItem(tr("会议室 A"), "room_a");
    roomCombo->addItem(tr("会议室 B"), "room_b");
    attendeesForm->addRow(tr("会议室"), roomCombo);

    attendeesSection_->setContent(attendeesContent);
    scrollLayout->addWidget(attendeesSection_);

    securitySection_ = new CollapsibleSection(tr("会议安全"), this);
    securitySection_->setExpanded(false);
    QWidget *securityContent = new QWidget();
    QFormLayout *securityForm = new QFormLayout(securityContent);
    securityForm->setSpacing(12);

    QCheckBox *passwordCheck = new QCheckBox(tr("启用会议密码"));
    passwordCheck->setObjectName("passwordCheck");
    securityForm->addRow(passwordCheck);

    QLineEdit *passwordEdit = new QLineEdit();
    passwordEdit->setObjectName("passwordEdit");
    passwordEdit->setPlaceholderText(tr("4-10位数字"));
    passwordEdit->setEnabled(false);
    passwordEdit->setEchoMode(QLineEdit::Password);
    securityForm->addRow(tr("密码"), passwordEdit);
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
    securityForm->addRow(tr("入会权限"), permissionCombo);

    securitySection_->setContent(securityContent);
    scrollLayout->addWidget(securitySection_);

    settingsSection_ = new CollapsibleSection(tr("会议设置"), this);
    settingsSection_->setExpanded(false);
    QWidget *settingsContent = new QWidget();
    QFormLayout *settingsForm = new QFormLayout(settingsContent);
    settingsForm->setSpacing(12);

    QCheckBox *muteCheck = new QCheckBox(tr("参会者入会自动静音"));
    muteCheck->setObjectName("muteCheck");
    muteCheck->setChecked(true);
    settingsForm->addRow(muteCheck);

    QComboBox *shareCombo = new QComboBox();
    shareCombo->setObjectName("shareCombo");
    shareCombo->addItem(tr("所有人"), 0);
    shareCombo->addItem(tr("仅主持人"), 1);
    settingsForm->addRow(tr("屏幕共享权限"), shareCombo);

    QComboBox *recordCombo = new QComboBox();
    recordCombo->setObjectName("recordCombo");
    recordCombo->addItem(tr("仅主持人"), 0);
    recordCombo->addItem(tr("所有人"), 1);
    settingsForm->addRow(tr("允许录制"), recordCombo);

    settingsSection_->setContent(settingsContent);
    scrollLayout->addWidget(settingsSection_);

    advancedSection_ = new CollapsibleSection(tr("高级选项"), this);
    advancedSection_->setExpanded(false);
    QWidget *advancedContent = new QWidget();
    QFormLayout *advancedForm = new QFormLayout(advancedContent);
    advancedForm->setSpacing(12);

    QComboBox *numberTypeCombo = new QComboBox();
    numberTypeCombo->setObjectName("numberTypeCombo");
    numberTypeCombo->addItem(tr("自动生成"), 0);
    numberTypeCombo->addItem(tr("使用个人会议号"), 1);
    advancedForm->addRow(tr("会议号"), numberTypeCombo);

    QTextEdit *descEdit = new QTextEdit();
    descEdit->setObjectName("descEdit");
    descEdit->setPlaceholderText(tr("添加会议描述（可选）"));
    descEdit->setMaximumHeight(80);
    advancedForm->addRow(tr("描述"), descEdit);

    advancedSection_->setContent(advancedContent);
    scrollLayout->addWidget(advancedSection_);

    scrollLayout->addStretch();
}

void BookMeeting::setupConnections()
{
    connect(ui->cancelBtn, &QPushButton::clicked, this, &BookMeeting::onCancelClicked);
    connect(ui->saveDraftBtn, &QPushButton::clicked, this, &BookMeeting::onSaveDraftClicked);
    connect(ui->bookNowBtn, &QPushButton::clicked, this, &BookMeeting::onBookNowClicked);
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
    QLineEdit *topicEdit = findChild<QLineEdit*>("topicEdit");
    if (!topicEdit || topicEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("验证失败"), tr("请输入会议主题"));
        basicInfoSection_->setExpanded(true);
        topicEdit->setFocus();
        return false;
    }
    QDateEdit *dateEdit = findChild<QDateEdit*>("dateEdit");
    QTimeEdit *timeEdit = findChild<QTimeEdit*>("timeEdit");
    if (dateEdit && timeEdit) {
        QDateTime selected(dateEdit->date(), timeEdit->time());
        if (selected < QDateTime::currentDateTime()) {
            QMessageBox::warning(this, tr("验证失败"), tr("开始时间必须晚于当前时间"));
            basicInfoSection_->setExpanded(true);
            return false;
        }
    }
    return true;
}

MeetingBookingInfo BookMeeting::getBookingInfo() const
{
    MeetingBookingInfo info;
    if (QLineEdit *topicEdit = findChild<QLineEdit*>("topicEdit"))
        info.topic = topicEdit->text();
    if (QDateEdit *dateEdit = findChild<QDateEdit*>("dateEdit"))
        if (QTimeEdit *timeEdit = findChild<QTimeEdit*>("timeEdit"))
            info.startTime = QDateTime(dateEdit->date(), timeEdit->time());
    if (QComboBox *durationCombo = findChild<QComboBox*>("durationCombo"))
        info.durationMinutes = durationCombo->currentData().toInt();
    if (QComboBox *tzCombo = findChild<QComboBox*>("timezoneCombo"))
        info.timeZone = tzCombo->currentData().toString();
    if (QLineEdit *emailsEdit = findChild<QLineEdit*>("emailsEdit"))
        if (!emailsEdit->text().isEmpty())
            info.inviteEmails = emailsEdit->text().split(QRegularExpression("[,;]"), Qt::SkipEmptyParts);
    if (QComboBox *roomCombo = findChild<QComboBox*>("roomCombo"))
        info.roomResource = roomCombo->currentData().toString();
    if (QCheckBox *passwordCheck = findChild<QCheckBox*>("passwordCheck"))
        if (QLineEdit *passwordEdit = findChild<QLineEdit*>("passwordEdit")) {
            info.passwordEnabled = passwordCheck->isChecked();
            info.password = passwordEdit->text();
        }
    if (QCheckBox *waitingRoomCheck = findChild<QCheckBox*>("waitingRoomCheck"))
        info.waitingRoomEnabled = waitingRoomCheck->isChecked();
    if (QComboBox *permissionCombo = findChild<QComboBox*>("permissionCombo"))
        info.joinPermission = permissionCombo->currentData().toInt();
    if (QCheckBox *muteCheck = findChild<QCheckBox*>("muteCheck"))
        info.autoMuteOnEntry = muteCheck->isChecked();
    if (QComboBox *shareCombo = findChild<QComboBox*>("shareCombo"))
        info.screenSharePermission = shareCombo->currentData().toInt();
    if (QComboBox *recordCombo = findChild<QComboBox*>("recordCombo"))
        info.recordingPermission = recordCombo->currentData().toInt();
    if (QComboBox *numberTypeCombo = findChild<QComboBox*>("numberTypeCombo"))
        info.meetingNumberType = numberTypeCombo->currentData().toInt();
    if (QTextEdit *descEdit = findChild<QTextEdit*>("descEdit"))
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
    if (QLineEdit *topicEdit = findChild<QLineEdit*>("topicEdit"))
        topicEdit->setText(settings.value("draft/topic").toString());
    if (QDateEdit *dateEdit = findChild<QDateEdit*>("dateEdit"))
        dateEdit->setDate(settings.value("draft/startTime").toDateTime().date());
    if (QTimeEdit *timeEdit = findChild<QTimeEdit*>("timeEdit"))
        timeEdit->setTime(settings.value("draft/startTime").toDateTime().time());
    if (QComboBox *durationCombo = findChild<QComboBox*>("durationCombo")) {
        int index = durationCombo->findData(settings.value("draft/duration", 60).toInt());
        if (index >= 0) durationCombo->setCurrentIndex(index);
    }
    if (QLineEdit *emailsEdit = findChild<QLineEdit*>("emailsEdit"))
        emailsEdit->setText(settings.value("draft/emails").toString());
}

bool BookMeeting::hasDraft() const
{
    QSettings settings("MeetEx", "BookMeeting");
    return settings.value("draft/hasData", false).toBool();
}

void BookMeeting::updateSectionSummaries()
{
    QLineEdit *topicEdit = findChild<QLineEdit*>("topicEdit");
    if (topicEdit && basicInfoSection_) {
        basicInfoSection_->setSummary(topicEdit->text());
    }
}

void BookMeeting::setBookingInfo(const MeetingBookingInfo &info)
{
    if (QLineEdit *topicEdit = findChild<QLineEdit*>("topicEdit"))
        topicEdit->setText(info.topic);
    if (QDateEdit *dateEdit = findChild<QDateEdit*>("dateEdit"))
        dateEdit->setDate(info.startTime.date());
    if (QTimeEdit *timeEdit = findChild<QTimeEdit*>("timeEdit"))
        timeEdit->setTime(info.startTime.time());
    if (QComboBox *durationCombo = findChild<QComboBox*>("durationCombo")) {
        int index = durationCombo->findData(info.durationMinutes);
        if (index >= 0) durationCombo->setCurrentIndex(index);
    }
}
