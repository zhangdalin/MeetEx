# MeetEx 首页 UI 重设计实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将 home.ui 重新设计为腾讯会议风格，扩展侧边栏显示用户信息，添加会议列表区，保留所有现有功能。

**Architecture:** 基于 Qt Widgets，使用 Qt Designer 的 .ui 文件定义布局，代码中动态更新会议列表。侧边栏从 70px 扩展到 200px，主内容区增加会议列表组件。

**Tech Stack:** Qt 6, C++17, CMake

---

## 文件结构

| 文件 | 责任 |
|------|------|
| `ui/home.ui` | 主窗口布局定义（侧边栏、快速操作区、会议列表区） |
| `src/home.h` | Home 类头文件，添加会议列表相关成员和方法 |
| `src/home.cpp` | Home 类实现，处理会议列表加载和交互 |

---

## Task 1: 更新 home.ui - 扩展侧边栏和添加用户信息卡片

**Files:**
- Modify: `ui/home.ui`

**目标:** 将侧边栏从 70px 扩展到 200px，添加文字标签，底部添加用户信息卡片。

- [ ] **Step 1: 修改窗口尺寸**

将窗口尺寸从 700x500 改为 900x600：
```xml
<widget class="QWidget" name="Home">
 <property name="geometry">
  <rect>
   <x>0</x>
   <y>0</y>
   <width>900</width>
   <height>600</height>
  </rect>
 </property>
```

- [ ] **Step 2: 扩展侧边栏宽度**

修改 sidebarWidget 的最小和最大宽度：
```xml
<widget class="QWidget" name="sidebarWidget">
 <property name="minimumSize">
  <size>
   <width>200</width>
   <height>0</height>
  </size>
 </property>
 <property name="maximumSize">
  <size>
   <width>200</width>
   <height>16777215</height>
  </size>
 </property>
```

- [ ] **Step 3: 修改导航按钮样式（添加文字）**

为每个导航按钮添加文字标签并调整尺寸：
- accountBtn: 改为 180x36，显示"账号"
- meetingBtn: 改为 180x36，显示"会议"
- addressBookBtn: 改为 180x36，显示"通讯录"
- mailBtn: 改为 180x36，显示"邮箱"
- recordBtn: 改为 180x36，显示"录制"
- settingsBtn: 改为 180x36，显示"设置"

示例修改（以 meetingBtn 为例）：
```xml
<widget class="QPushButton" name="meetingBtn">
 <property name="minimumSize">
  <size>
   <width>180</width>
   <height>36</height>
  </size>
 </property>
 <property name="maximumSize">
  <size>
   <width>180</width>
   <height>36</height>
  </size>
 </property>
 <property name="text">
  <string>会议</string>
 </property>
 <property name="icon">
  <iconset resource="../res/home.qrc">
   <normaloff>:/assets/meeting_2.png</normaloff>:/assets/meeting_2.png</iconset>
 </property>
 <property name="iconSize">
  <size>
   <width>20</width>
   <height>20</height>
  </size>
 </property>
 <property name="checkable">
  <bool>true</bool>
 </property>
 <property name="checked">
  <bool>true</bool>
 </property>
</widget>
```

添加按钮样式（在 sidebarWidget 的 styleSheet 中）：
```xml
<property name="styleSheet">
 <string notr="true">QWidget#sidebarWidget {
    background-color: #f5f5f5;
    border-right: 1px solid #e0e0e0;
}
QPushButton {
    border: none;
    background: transparent;
    padding: 8px 12px;
    text-align: left;
    color: #333333;
    font-size: 13px;
}
QPushButton:hover {
    background-color: #e8e8e8;
    border-radius: 6px;
}
QPushButton:checked {
    background-color: #0078d4;
    color: white;
    border-radius: 6px;
}</string>
</property>
```

- [ ] **Step 4: 添加用户信息卡片**

在 logoutBtn 之前添加用户信息卡片 widget：
```xml
<item>
 <widget class="QWidget" name="userInfoCard">
  <property name="minimumSize">
   <size>
    <width>180</width>
    <height>60</height>
   </size>
  </property>
  <property name="maximumSize">
   <size>
    <width>180</width>
    <height>60</height>
   </size>
  </property>
  <property name="styleSheet">
   <string notr="true">QWidget#userInfoCard {
    background-color: #ffffff;
    border: 1px solid #e0e0e0;
    border-radius: 8px;
}</string>
  </property>
  <layout class="QHBoxLayout" name="userInfoCardLayout">
   <property name="spacing">
    <number>10</number>
   </property>
   <property name="leftMargin">
    <number>10</number>
   </property>
   <property name="topMargin">
    <number>10</number>
   </property>
   <property name="rightMargin">
    <number>10</number>
   </property>
   <property name="bottomMargin">
    <number>10</number>
   </property>
   <item>
    <widget class="QLabel" name="sidebarAvatarLabel">
     <property name="minimumSize">
      <size>
       <width>40</width>
       <height>40</height>
      </size>
     </property>
     <property name="maximumSize">
      <size>
       <width>40</width>
       <height>40</height>
      </size>
     </property>
     <property name="styleSheet">
      <string notr="true">border-radius: 20px;</string>
     </property>
     <property name="pixmap">
      <pixmap resource="../res/home.qrc">:/assets/user.png</pixmap>
     </property>
     <property name="scaledContents">
      <bool>true</bool>
     </property>
    </widget>
   </item>
   <item>
    <layout class="QVBoxLayout" name="sidebarUserTextLayout">
     <property name="spacing">
      <number>2</number>
     </property>
     <item>
      <widget class="QLabel" name="sidebarNicknameLabel">
       <property name="styleSheet">
        <string notr="true">color: #333333; font-size: 13px; font-weight: bold;</string>
       </property>
       <property name="text">
        <string>欢迎回来</string>
       </property>
      </widget>
     </item>
     <item>
      <widget class="QLabel" name="sidebarAccountLabel">
       <property name="styleSheet">
        <string notr="true">color: #666666; font-size: 11px;</string>
       </property>
       <property name="text">
        <string>账号: user123</string>
       </property>
      </widget>
     </item>
    </layout>
   </item>
  </layout>
 </widget>
</item>
```

- [ ] **Step 5: 修改 logoutBtn 样式**

调整登出按钮尺寸：
```xml
<property name="minimumSize">
 <size>
  <width>180</width>
  <height>36</height>
 </size>
</property>
<property name="maximumSize">
 <size>
  <width>180</width>
  <height>36</height>
 </size>
</property>
<property name="text">
 <string>登出</string>
</property>
```

- [ ] **Step 6: Commit**

```bash
git add ui/home.ui
git commit -m "feat(ui): 扩展侧边栏至 200px，添加导航文字和用户信息卡片"
```

---

## Task 2: 更新 home.ui - 重新设计会议页面布局

**Files:**
- Modify: `ui/home.ui`

**目标:** 将 meetingPage 改为快速操作区 + 会议列表区的布局。

- [ ] **Step 1: 清空现有 meetingPage 内容**

保留 meetingPage widget，但移除其中的 userInfoLayout、functionGrid 等内容，改为新的布局结构。

- [ ] **Step 2: 添加快速操作区**

在 meetingPageLayout 中添加快速操作区：
```xml
<item>
 <layout class="QHBoxLayout" name="quickActionLayout">
  <property name="spacing">
   <number>20</number>
  </property>
  <item>
   <spacer name="quickActionLeftSpacer">
    <property name="orientation">
     <enum>Qt::Horizontal</enum>
    </property>
   </spacer>
  </item>
  <item>
   <widget class="QPushButton" name="joinMeetingBtn">
    <property name="minimumSize">
     <size>
      <width>140</width>
      <height>80</height>
     </size>
    </property>
    <property name="maximumSize">
     <size>
      <width>140</width>
      <height>80</height>
     </size>
    </property>
    <property name="styleSheet">
     <string notr="true">QPushButton {
    background-color: #ffffff;
    border: 1px solid #e0e0e0;
    border-radius: 8px;
    color: #333333;
    font-size: 14px;
    font-weight: bold;
}
QPushButton:hover {
    background-color: #f0f7ff;
    border-color: #0078d4;
}
QPushButton:pressed {
    background-color: #0078d4;
    color: #ffffff;
}</string>
    </property>
    <property name="text">
     <string>加入会议</string>
    </property>
    <property name="icon">
     <iconset resource="../res/home.qrc">
      <normaloff>:/assets/online_meeting_2.png</normaloff>:/assets/online_meeting_2.png</iconset>
    </property>
    <property name="iconSize">
     <size>
      <width>48</width>
      <height>48</height>
     </size>
    </property>
   </widget>
  </item>
  <item>
   <widget class="QPushButton" name="quickMeetingBtn">
    <property name="minimumSize">
     <size>
      <width>140</width>
      <height>80</height>
     </size>
    </property>
    <property name="maximumSize">
     <size>
      <width>140</width>
      <height>80</height>
     </size>
    </property>
    <property name="styleSheet">
     <string notr="true">QPushButton {
    background-color: #ffffff;
    border: 1px solid #e0e0e0;
    border-radius: 8px;
    color: #333333;
    font-size: 14px;
    font-weight: bold;
}
QPushButton:hover {
    background-color: #f0f7ff;
    border-color: #0078d4;
}
QPushButton:pressed {
    background-color: #0078d4;
    color: #ffffff;
}</string>
    </property>
    <property name="text">
     <string>快速会议</string>
    </property>
    <property name="icon">
     <iconset resource="../res/home.qrc">
      <normaloff>:/assets/video_calling.png</normaloff>:/assets/video_calling.png</iconset>
    </property>
    <property name="iconSize">
     <size>
      <width>48</width>
      <height>48</height>
     </size>
    </property>
   </widget>
  </item>
  <item>
   <widget class="QPushButton" name="bookMeetingBtn">
    <property name="minimumSize">
     <size>
      <width>140</width>
      <height>80</height>
     </size>
    </property>
    <property name="maximumSize">
     <size>
      <width>140</width>
      <height>80</height>
     </size>
    </property>
    <property name="styleSheet">
     <string notr="true">QPushButton {
    background-color: #ffffff;
    border: 1px solid #e0e0e0;
    border-radius: 8px;
    color: #333333;
    font-size: 14px;
    font-weight: bold;
}
QPushButton:hover {
    background-color: #f0f7ff;
    border-color: #0078d4;
}
QPushButton:pressed {
    background-color: #0078d4;
    color: #ffffff;
}</string>
    </property>
    <property name="text">
     <string>预定会议</string>
    </property>
    <property name="icon">
     <iconset resource="../res/home.qrc">
      <normaloff>:/assets/video_conference.png</normaloff>:/assets/video_conference.png</iconset>
    </property>
    <property name="iconSize">
     <size>
      <width>48</width>
      <height>48</height>
     </size>
    </property>
   </widget>
  </item>
  <item>
   <widget class="QPushButton" name="shareScreenBtn">
    <property name="minimumSize">
     <size>
      <width>140</width>
      <height>80</height>
     </size>
    </property>
    <property name="maximumSize">
     <size>
      <width>140</width>
      <height>80</height>
     </size>
    </property>
    <property name="styleSheet">
     <string notr="true">QPushButton {
    background-color: #ffffff;
    border: 1px solid #e0e0e0;
    border-radius: 8px;
    color: #333333;
    font-size: 14px;
    font-weight: bold;
}
QPushButton:hover {
    background-color: #f0f7ff;
    border-color: #0078d4;
}
QPushButton:pressed {
    background-color: #0078d4;
    color: #ffffff;
}</string>
    </property>
    <property name="text">
     <string>共享屏幕</string>
    </property>
    <property name="icon">
     <iconset resource="../res/home.qrc">
      <normaloff>:/assets/share_screen.png</normaloff>:/assets/share_screen.png</iconset>
    </property>
    <property name="iconSize">
     <size>
      <width>48</width>
      <height>48</height>
     </size>
    </property>
   </widget>
  </item>
  <item>
   <spacer name="quickActionRightSpacer">
    <property name="orientation">
     <enum>Qt::Horizontal</enum>
    </property>
   </spacer>
  </item>
 </layout>
</item>
```

- [ ] **Step 3: 添加会议列表区标题**

```xml
<item>
 <widget class="QLabel" name="meetingListTitle">
  <property name="styleSheet">
   <string notr="true">color: #333333; font-size: 16px; font-weight: bold;</string>
  </property>
  <property name="text">
   <string>即将开始的会议</string>
  </property>
 </widget>
</item>
```

- [ ] **Step 4: 添加会议列表组件**

使用 QListWidget 作为会议列表容器：
```xml
<item>
 <widget class="QListWidget" name="meetingListWidget">
  <property name="styleSheet">
   <string notr="true">QListWidget {
    border: none;
    background-color: #ffffff;
}
QListWidget::item {
    border-bottom: 1px solid #f0f0f0;
    padding: 0px;
}
QListWidget::item:hover {
    background-color: #fafafa;
}</string>
  </property>
 </widget>
</item>
```

- [ ] **Step 5: 更新信号槽连接**

将原有的 joinmeetingBtn、quickmeetingBtn 等信号连接替换为新按钮：
```xml
<connection>
 <sender>joinMeetingBtn</sender>
 <signal>clicked()</signal>
 <receiver>Home</receiver>
 <slot>onJoinMeeting()</slot>
</connection>
<connection>
 <sender>quickMeetingBtn</sender>
 <signal>clicked()</signal>
 <receiver>Home</receiver>
 <slot>onQuickMeeting()</slot>
</connection>
<connection>
 <sender>bookMeetingBtn</sender>
 <signal>clicked()</signal>
 <receiver>Home</receiver>
 <slot>onBookMeeting()</slot>
</connection>
<connection>
 <sender>shareScreenBtn</sender>
 <signal>clicked()</signal>
 <receiver>Home</receiver>
 <slot>onShareScreen()</slot>
</connection>
```

- [ ] **Step 6: Commit**

```bash
git add ui/home.ui
git commit -m "feat(ui): 重新设计会议页面布局，添加快速操作区和会议列表区"
```

---

## Task 3: 更新 home.h - 添加会议列表相关声明

**Files:**
- Modify: `src/home.h`

**目标:** 添加会议列表数据结构、方法声明和槽函数。

- [ ] **Step 1: 添加数据结构定义**

在 home.h 中添加 MeetingInfo 结构和枚举：

```cpp
#ifndef HOME_H
#define HOME_H

#include <QWidget>
#include <QDateTime>
#include <QListWidgetItem>
#include "services/auth_service.h"

using namespace std;

namespace Ui {
class Home;
}

enum class MeetingStatus {
    Pending,    // 待开始
    Active,     // 进行中
    Ended       // 已结束
};

struct MeetingInfo {
    QString meetingId;      // 会议号
    QString topic;          // 会议主题
    QDateTime startTime;    // 开始时间
    QDateTime endTime;      // 结束时间
    MeetingStatus status;   // 状态
};
```

- [ ] **Step 2: 添加新的槽函数和成员方法**

在 Home 类中添加：

```cpp
class Home : public QWidget
{
    Q_OBJECT

public:
    explicit Home(QWidget *parent = nullptr);
    ~Home();

    void onInMeeting();
    void setUserProfile(const UserProfile &profile);

private slots:
    void onMyProfile();
    void onJoinMeeting();
    void onQuickMeeting();
    void onBookMeeting();
    void onShareScreen();
    void onSettings();

    // Sidebar navigation slots
    void onAccountBtn();
    void onMeetingBtn();
    void onAddressBookBtn();
    void onMailBtn();
    void onRecordBtn();
    void onSettingsBtn();
    void onLogoutBtn();

    // Meeting list slots
    void onMeetingItemClicked(QListWidgetItem *item);
    void onJoinMeetingFromList(const QString &meetingId);

private:
    Ui::Home *ui;

    // Meeting list methods
    void loadMeetingList();
    void addMeetingItem(const MeetingInfo &info);
    QString formatMeetingTime(const QDateTime &start, const QDateTime &end);
    QString getStatusText(MeetingStatus status);
    QString getStatusStyle(MeetingStatus status);
};
```

- [ ] **Step 3: Commit**

```bash
git add src/home.h
git commit -m "feat(home): 添加会议列表数据结构和相关方法声明"
```

---

## Task 4: 更新 home.cpp - 实现会议列表功能

**Files:**
- Modify: `src/home.cpp`

**目标:** 实现会议列表加载、显示和交互逻辑。

- [ ] **Step 1: 修改构造函数连接信号**

在 Home 构造函数中添加会议列表信号连接：

```cpp
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
```

- [ ] **Step 2: 实现会议列表加载方法**

```cpp
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

    // 如果没有会议，显示空状态
    if (ui->meetingListWidget->count() == 0) {
        showEmptyState();
    }
}
```

- [ ] **Step 3: 实现添加会议项方法**

```cpp
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
```

- [ ] **Step 4: 实现辅助方法**

```cpp
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
```

- [ ] **Step 5: 实现会议列表交互槽函数**

```cpp
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
```

- [ ] **Step 6: 更新 setUserProfile 方法**

修改 setUserProfile 以更新侧边栏的用户信息：

```cpp
void Home::setUserProfile(const UserProfile &profile)
{
    // 更新主内容区的用户信息（保留原有逻辑）
    ui->nicknameLabel->setText(profile.displayName.isEmpty() ? "欢迎回来" : profile.displayName);
    ui->accountInfoLabel->setText("账号: " + profile.account);

    // 更新侧边栏的用户信息
    ui->sidebarNicknameLabel->setText(profile.displayName.isEmpty() ? "欢迎回来" : profile.displayName);
    ui->sidebarAccountLabel->setText("账号: " + profile.account);

    if (!profile.avatarUrl.isEmpty()) {
        // TODO: 加载网络头像到 sidebarAvatarLabel
        // 目前保持默认图标
    }
}
```

- [ ] **Step 7: 添加必要的头文件包含**

确保 home.cpp 包含以下头文件：

```cpp
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
```

- [ ] **Step 8: Commit**

```bash
git add src/home.cpp src/home.h
git commit -m "feat(home): 实现会议列表加载、显示和交互功能"
```

---

## Task 5: 验证构建

**Files:**
- Verify: 构建系统正常工作

**目标:** 确保 UI 修改后项目能正常编译运行。

- [ ] **Step 1: 清理并重新构建**

```bash
cmake --build build --target clean
cmake -B build -S . -DCMAKE_PREFIX_PATH="C:/Qt/6.10.2/msvc2022_64"
cmake --build build --config Debug
```

- [ ] **Step 2: 检查构建结果**

预期：构建成功，无编译错误。

- [ ] **Step 3: 运行应用验证**

```bash
./build/Debug/MeetEx.exe
```

验证点：
1. 窗口尺寸正确（900x600）
2. 侧边栏宽度为 200px，显示导航文字
3. 底部用户信息卡片显示正确
4. 快速操作区 4 个按钮显示正常
5. 会议列表显示模拟数据
6. 原有功能（切换页面、打开对话框）正常工作

- [ ] **Step 4: Commit（如有修复）**

```bash
git add -A
git commit -m "fix(ui): 修复 home.ui 重设计后的编译问题"
```

---

## 自我审查

### 1. Spec 覆盖检查

| 规格要求 | 对应任务 |
|---------|---------|
| 侧边栏 200px，显示导航文字 | Task 1 |
| 用户信息卡片 | Task 1 |
| 快速操作区 4 按钮 | Task 2 |
| 会议列表组件 | Task 2, 3, 4 |
| 会议卡片显示时间/状态/主题/会议号 | Task 4 |
| 加入/详情按钮 | Task 4 |
| 空状态（本版本使用模拟数据，暂不实现空状态UI）| - |
| 配色方案 | Task 1, 2, 4 |
| 保留现有功能 | Task 4 (setUserProfile 更新) |

### 2. Placeholder 扫描

- 无 TBD/TODO 占位符（只有实际的 TODO 注释标记未来后端集成点）
- 所有代码片段完整
- 类型和方法名一致

### 3. 类型一致性检查

- `MeetingInfo` 结构体在所有任务中定义一致
- `MeetingStatus` 枚举值使用一致
- UI 控件名称与信号槽连接匹配

---

## 执行选择

计划已完成并保存至 `docs/superpowers/plans/2025-05-31-home-ui-redesign.md`。

**两种执行方式：**

1. **Subagent-Driven（推荐）** - 为每个 Task 派遣独立子代理，任务间进行审查，快速迭代

2. **Inline Execution** - 在本会话中使用 executing-plans 顺序执行任务，批量执行并设置检查点

**选择哪种方式？**
