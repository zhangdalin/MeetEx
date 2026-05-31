#include "home.h"
#include "ui_home.h"
#include "myprofile.h"
#include "joinmeeting.h"
#include "inmeeting.h"
#include "bookmeeting.h"
#include "sharescreen.h"
#include "settings.h"
#include "login.h"

#include <QMessageBox>
#include <QThread>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

using namespace std;

unique_ptr<QWidget> myprofile = nullptr;
unique_ptr<QWidget> joinmeeting = nullptr;
unique_ptr<QWidget> inmeeting = nullptr;
unique_ptr<QWidget> bookmeeting = nullptr;
unique_ptr<QWidget> sharescreen = nullptr;
unique_ptr<QWidget> settings = nullptr;

Home::Home(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Home)
{
    ui->setupUi(this);

    // 连接会议列表点击信号
    connect(ui->meetingListWidget, &QListWidget::itemClicked,
            this, &Home::onMeetingItemClicked);

    // 初始加载会议列表（模拟数据）
    loadMeetingList();
}

Home::~Home()
{
    delete ui;
}

void Home::onMyProfile()
{
    if (!myprofile) {
        myprofile = make_unique<MyProfile>();
        myprofile->show();
        connect(static_cast<MyProfile*>(myprofile.get()), &MyProfile::sigClosing, this, [this]{
            qInfo() << QThread::currentThread() << __FUNCTION__ << "my profile windows closed";
            myprofile.reset();
        });
    }
    else {
        myprofile->activateWindow();
    }
}

void Home::onJoinMeeting()
{
    if (!joinmeeting) {
        joinmeeting = make_unique<JoinMeeting>();
        joinmeeting->show();
        connect(static_cast<JoinMeeting*>(joinmeeting.get()), &JoinMeeting::sigClosing, this, [this]{
            qInfo() << QThread::currentThread() << __FUNCTION__ << "join meeting windows closed";
            joinmeeting.reset();
        });
    }
    else {
        joinmeeting->activateWindow();
    }
}

void Home::onInMeeting()
{
    if (!inmeeting) {
        inmeeting = make_unique<InMeeting>();
        inmeeting->show();
        connect(static_cast<InMeeting*>(inmeeting.get()), &InMeeting::sigClosing, this, [this]{
            qInfo() << QThread::currentThread() << __FUNCTION__ << "in meeting windows closed";
            inmeeting.reset();
        });
    }
    else {
        inmeeting->activateWindow();
    }
}

void Home::onQuickMeeting()
{
    // todo
    // add some logic
    onInMeeting();
}

void Home::onBookMeeting()
{
    if (!bookmeeting) {
        bookmeeting = make_unique<BookMeeting>();
        bookmeeting->show();
        connect(static_cast<BookMeeting*>(bookmeeting.get()), &BookMeeting::sigClosing, this, [this]{
            qInfo() << QThread::currentThread() << __FUNCTION__ << "book meeting windows closed";
            bookmeeting.reset();
        });
    }
    else {
        bookmeeting->activateWindow();
    }
}

void Home::onShareScreen()
{
    if (!sharescreen) {
        sharescreen = make_unique<ShareScreen>();
        sharescreen->show();
        connect(static_cast<ShareScreen*>(sharescreen.get()), &ShareScreen::sigClosing, this, [this]{
            qInfo() << QThread::currentThread() << __FUNCTION__ << "share screen windows closed";
            sharescreen.reset();
        });
    }
    else {
        sharescreen->activateWindow();
    }
}

void Home::onSettings()
{
    if (!settings) {
        settings = make_unique<Settings>();
        settings->show();
        connect(static_cast<Settings*>(settings.get()), &Settings::sigClosing, this, [this]{
            qInfo() << QThread::currentThread() << __FUNCTION__ << "settings windows closed";
            settings.reset();
        });
    }
    else {
        settings->activateWindow();
    }
}

void Home::onAccountBtn()
{
    ui->stackedWidget->setCurrentWidget(ui->accountPage);
    ui->accountBtn->setChecked(true);
    ui->meetingBtn->setChecked(false);
    ui->addressBookBtn->setChecked(false);
    ui->mailBtn->setChecked(false);
    ui->recordBtn->setChecked(false);
    ui->settingsBtn->setChecked(false);
}

void Home::onMeetingBtn()
{
    ui->stackedWidget->setCurrentWidget(ui->meetingPage);
    ui->accountBtn->setChecked(false);
    ui->meetingBtn->setChecked(true);
    ui->addressBookBtn->setChecked(false);
    ui->mailBtn->setChecked(false);
    ui->recordBtn->setChecked(false);
    ui->settingsBtn->setChecked(false);
}

void Home::onAddressBookBtn()
{
    ui->stackedWidget->setCurrentWidget(ui->addressBookPage);
    ui->accountBtn->setChecked(false);
    ui->meetingBtn->setChecked(false);
    ui->addressBookBtn->setChecked(true);
    ui->mailBtn->setChecked(false);
    ui->recordBtn->setChecked(false);
    ui->settingsBtn->setChecked(false);
}

void Home::onMailBtn()
{
    ui->stackedWidget->setCurrentWidget(ui->mailPage);
    ui->accountBtn->setChecked(false);
    ui->meetingBtn->setChecked(false);
    ui->addressBookBtn->setChecked(false);
    ui->mailBtn->setChecked(true);
    ui->recordBtn->setChecked(false);
    ui->settingsBtn->setChecked(false);
}

void Home::onRecordBtn()
{
    ui->stackedWidget->setCurrentWidget(ui->recordPage);
    ui->accountBtn->setChecked(false);
    ui->meetingBtn->setChecked(false);
    ui->addressBookBtn->setChecked(false);
    ui->mailBtn->setChecked(false);
    ui->recordBtn->setChecked(true);
    ui->settingsBtn->setChecked(false);
}

void Home::onSettingsBtn()
{
    // Open settings dialog instead of switching page
    onSettings();
}

void Home::setUserProfile(const UserProfile &profile)
{
    // 更新侧边栏的用户信息
    ui->sidebarNicknameLabel->setText(profile.displayName.isEmpty() ? "欢迎回来" : profile.displayName);
    ui->sidebarAccountLabel->setText("账号: " + profile.account);

    if (!profile.avatarUrl.isEmpty()) {
        // TODO: 加载网络头像到 sidebarAvatarLabel
        // 目前保持默认图标
    }
}

void Home::onLogoutBtn()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this, "确认登出",
        "确定要退出登录吗？",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // 调用认证服务登出
        AuthService::instance().logout();

        // 创建新的登录窗口
        Login *loginWindow = new Login();
        loginWindow->show();

        // 关闭当前 Home 窗口
        close();
    }
}

void Home::loadMeetingList()
{
    ui->meetingListWidget->clear();

    // TODO: 从后端服务获取真实数据
    // 当前使用模拟数据展示布局效果

    QDateTime now = QDateTime::currentDateTime();

    // 模拟进行中的会议
    MeetingInfo activeMeeting;
    activeMeeting.meetingId = "123 456 789";
    activeMeeting.topic = "团队周会";
    activeMeeting.startTime = now.addSecs(-1800);  // 30分钟前开始
    activeMeeting.endTime = now.addSecs(1800);     // 30分钟后结束
    activeMeeting.status = MeetingStatus::Active;
    addMeetingItem(activeMeeting);

    // 模拟待开始的会议
    MeetingInfo pendingMeeting;
    pendingMeeting.meetingId = "987 654 321";
    pendingMeeting.topic = "项目评审";
    pendingMeeting.startTime = now.addSecs(3600);  // 1小时后开始
    pendingMeeting.endTime = now.addSecs(7200);    // 2小时后结束
    pendingMeeting.status = MeetingStatus::Pending;
    addMeetingItem(pendingMeeting);

    // 模拟明天的会议
    MeetingInfo tomorrowMeeting;
    tomorrowMeeting.meetingId = "456 789 123";
    tomorrowMeeting.topic = "产品规划";
    tomorrowMeeting.startTime = now.addDays(1).addSecs(3600);
    tomorrowMeeting.endTime = now.addDays(1).addSecs(7200);
    tomorrowMeeting.status = MeetingStatus::Pending;
    addMeetingItem(tomorrowMeeting);
}

void Home::addMeetingItem(const MeetingInfo &info)
{
    // 创建列表项
    QListWidgetItem *item = new QListWidgetItem(ui->meetingListWidget);
    item->setData(Qt::UserRole, info.meetingId);
    item->setSizeHint(QSize(0, 70));

    // 创建自定义 widget 显示会议信息
    QWidget *meetingWidget = new QWidget();
    QHBoxLayout *mainLayout = new QHBoxLayout(meetingWidget);
    mainLayout->setContentsMargins(15, 10, 15, 10);
    mainLayout->setSpacing(15);

    // 时间区域
    QLabel *timeLabel = new QLabel(formatMeetingTime(info.startTime, info.endTime));
    timeLabel->setStyleSheet("color: #333333; font-size: 13px;");
    timeLabel->setFixedWidth(100);
    mainLayout->addWidget(timeLabel);

    // 信息区域（垂直布局）
    QVBoxLayout *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(4);
    infoLayout->setContentsMargins(0, 0, 0, 0);

    // 状态 + 主题
    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->setSpacing(8);

    QLabel *statusLabel = new QLabel(getStatusText(info.status));
    statusLabel->setStyleSheet(getStatusStyle(info.status));
    titleLayout->addWidget(statusLabel);

    QLabel *topicLabel = new QLabel(info.topic);
    topicLabel->setStyleSheet("color: #333333; font-size: 14px; font-weight: bold;");
    titleLayout->addWidget(topicLabel);
    titleLayout->addStretch();

    infoLayout->addLayout(titleLayout);

    // 会议号
    QLabel *idLabel = new QLabel("会议号: " + info.meetingId);
    idLabel->setStyleSheet("color: #666666; font-size: 12px;");
    infoLayout->addWidget(idLabel);

    mainLayout->addLayout(infoLayout, 1);

    // 操作区域
    QHBoxLayout *actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(8);

    if (info.status != MeetingStatus::Ended) {
        QPushButton *joinBtn = new QPushButton("加入");
        joinBtn->setStyleSheet(
            "QPushButton {"
            "  background-color: #0078d4;"
            "  color: white;"
            "  border: none;"
            "  border-radius: 4px;"
            "  padding: 6px 16px;"
            "  font-size: 12px;"
            "}"
            "QPushButton:hover {"
            "  background-color: #106ebe;"
            "}"
        );
        connect(joinBtn, &QPushButton::clicked, this, [this, info]() {
            onJoinMeetingFromList(info.meetingId);
        });
        actionLayout->addWidget(joinBtn);
    }

    QPushButton *detailBtn = new QPushButton("详情");
    detailBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: transparent;"
        "  color: #666666;"
        "  border: none;"
        "  font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "  color: #0078d4;"
        "}"
    );
    actionLayout->addWidget(detailBtn);

    mainLayout->addLayout(actionLayout);

    // 设置 item 的 widget
    ui->meetingListWidget->addItem(item);
    ui->meetingListWidget->setItemWidget(item, meetingWidget);
}

QString Home::formatMeetingTime(const QDateTime &start, const QDateTime &end)
{
    QString startStr = start.toString("HH:mm");
    QString endStr = end.toString("HH:mm");

    // 判断是否跨天
    QDate today = QDate::currentDate();
    QDate tomorrow = today.addDays(1);

    if (start.date() == today) {
        return startStr + " - " + endStr;
    } else if (start.date() == tomorrow) {
        return "明天 " + startStr + " - " + endStr;
    } else {
        return start.toString("MM-dd ") + startStr + " - " + endStr;
    }
}

QString Home::getStatusText(MeetingStatus status)
{
    switch (status) {
    case MeetingStatus::Pending:
        return "待开始";
    case MeetingStatus::Active:
        return "进行中";
    case MeetingStatus::Ended:
        return "已结束";
    default:
        return "";
    }
}

QString Home::getStatusStyle(MeetingStatus status)
{
    switch (status) {
    case MeetingStatus::Pending:
        return "color: #999999; font-size: 12px; background-color: #f5f5f5; "
               "padding: 2px 8px; border-radius: 4px;";
    case MeetingStatus::Active:
        return "color: #0078d4; font-size: 12px; background-color: #e6f2ff; "
               "padding: 2px 8px; border-radius: 4px;";
    case MeetingStatus::Ended:
        return "color: #999999; font-size: 12px; text-decoration: line-through; "
               "padding: 2px 8px; border-radius: 4px;";
    default:
        return "";
    }
}

void Home::onMeetingItemClicked(QListWidgetItem *item)
{
    QString meetingId = item->data(Qt::UserRole).toString();
    qInfo() << "Meeting item clicked:" << meetingId;
    // TODO: 打开会议详情页面或对话框
}

void Home::onJoinMeetingFromList(const QString &meetingId)
{
    qInfo() << "Join meeting from list:" << meetingId;
    // TODO: 预填充会议号并打开加入会议对话框
    // 或者直接进入会议
    onJoinMeeting();
}
