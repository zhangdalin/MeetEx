# BookMeeting 界面重设计实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 重写 BookMeeting 界面，使其风格与 Login/Register 保持一致，使用 QFormLayout 表单布局，添加输入框图标，分组采用"前两个展开+后三个可折叠"的组织方式。

**Architecture:** 通过 Qt Designer 重新设计 ui/bookmeeting.ui，使用 QFormLayout 实现表单布局；src/bookmeeting.cpp 简化为使用 ui 文件和添加图标；保留 CollapsibleSection 仅用于后三个分组。

**Tech Stack:** Qt 6, C++17, Qt Designer (.ui files)

---

## 文件结构

| 文件 | 操作 | 说明 |
|------|------|------|
| `ui/bookmeeting.ui` | 重写 | 完全重新设计，使用 QFormLayout，窗口尺寸 460×620 |
| `src/bookmeeting.h` | 修改 | 移除动态创建的控件指针，保留 CollapsibleSection 指针 |
| `src/bookmeeting.cpp` | 重写 | 简化实现，基于 ui 文件，添加图标，设置样式 |
| `src/collapsiblesection.h` | 确认 | 确保可折叠分组组件可用（复用现有） |

---

## Task 1: 备份当前文件

**Files:**
- 备份: `ui/bookmeeting.ui` → `ui/bookmeeting.ui.bak`
- 备份: `src/bookmeeting.cpp` → `src/bookmeeting.cpp.bak`

- [ ] **Step 1: 备份现有 UI 文件**

```bash
cp ui/bookmeeting.ui ui/bookmeeting.ui.bak
cp src/bookmeeting.cpp src/bookmeeting.cpp.bak
```

- [ ] **Step 2: 确认备份成功**

```bash
ls -la ui/bookmeeting.ui.bak src/bookmeeting.cpp.bak
```

Expected: 两个备份文件存在

---

## Task 2: 重写 ui/bookmeeting.ui

**Files:**
- 重写: `ui/bookmeeting.ui`

**设计规范：**
- 窗口尺寸: 460×620
- 主布局: QVBoxLayout，边距 25,20,25,20
- 分组1/2: QLabel 作为分组标题（灰色背景 #f5f5f5）
- 分组3/4/5: 预留占位，代码中动态添加 CollapsibleSection
- 底部按钮: 取消/保存草稿/立即预定

- [ ] **Step 1: 编写新的 ui/bookmeeting.ui**

```xml
<?xml version="1.0" encoding="UTF-8"?>
<ui version="4.0">
 <class>BookMeeting</class>
 <widget class="QWidget" name="BookMeeting">
  <property name="geometry">
   <rect>
    <x>0</x>
    <y>0</y>
    <width>460</width>
    <height>620</height>
   </rect>
  </property>
  <property name="minimumSize">
   <size>
    <width>420</width>
    <height>580</height>
   </size>
  </property>
  <property name="windowTitle">
   <string>预定会议</string>
  </property>
  <layout class="QVBoxLayout" name="mainLayout">
   <property name="spacing">
    <number>0</number>
   </property>
   <property name="leftMargin">
    <number>25</number>
   </property>
   <property name="topMargin">
    <number>20</number>
   </property>
   <property name="rightMargin">
    <number>25</number>
   </property>
   <property name="bottomMargin">
    <number>20</number>
   </property>

   <!-- 顶部间距 -->
   <item>
    <spacer name="topSpacer">
     <property name="orientation">
      <enum>Qt::Vertical</enum>
     </property>
    </spacer>
   </item>

   <!-- 标题 -->
   <item>
    <widget class="QLabel" name="titleLabel">
     <property name="font">
      <font>
       <pointsize>18</pointsize>
       <bold>true</bold>
      </font>
     </property>
     <property name="text">
      <string>预定会议</string>
     </property>
     <property name="alignment">
      <set>Qt::AlignCenter</set>
     </property>
    </widget>
   </item>

   <!-- 标题后间距 -->
   <item>
    <spacer name="afterTitleSpacer">
     <property name="orientation">
      <enum>Qt::Vertical</enum>
     </property>
     <property name="sizeHint" stdset="0">
      <size>
       <width>20</width>
       <height>20</height>
      </size>
     </property>
    </spacer>
   </item>

   <!-- 基本信息分组 -->
   <item>
    <widget class="QLabel" name="basicGroupLabel">
     <property name="font">
      <font>
       <pointsize>10</pointsize>
       <bold>true</bold>
      </font>
     </property>
     <property name="styleSheet">
      <string notr="true">QLabel {
    background-color: #f5f5f5;
    color: #666666;
    padding: 5px 15px;
    border-radius: 3px;
}</string>
     </property>
     <property name="text">
      <string>━━━ 基本信息 ━━━</string>
     </property>
     <property name="alignment">
      <set>Qt::AlignCenter</set>
     </property>
    </widget>
   </item>

   <!-- 基本信息表单 -->
   <item>
    <layout class="QFormLayout" name="basicFormLayout">
     <property name="labelAlignment">
      <set>Qt::AlignRight|Qt::AlignVCenter</set>
     </property>
     <property name="formAlignment">
      <set>Qt::AlignLeading|Qt::AlignTop</set>
     </property>
     <property name="horizontalSpacing">
      <number>10</number>
     </property>
     <property name="verticalSpacing">
      <number>12</number>
     </property>

     <!-- 会议主题 -->
     <item row="0" column="0">
      <widget class="QLabel" name="topicLabel">
       <property name="text">
        <string>会议主题:</string>
       </property>
      </widget>
     </item>
     <item row="0" column="1">
      <widget class="QLineEdit" name="topicEdit">
       <property name="placeholderText">
        <string>请输入会议主题</string>
       </property>
      </widget>
     </item>

     <!-- 开始时间 -->
     <item row="1" column="0">
      <widget class="QLabel" name="startTimeLabel">
       <property name="text">
        <string>开始时间:</string>
       </property>
      </widget>
     </item>
     <item row="1" column="1">
      <layout class="QHBoxLayout" name="startTimeLayout">
       <property name="spacing">
        <number>5</number>
       </property>
       <item>
        <widget class="QDateEdit" name="dateEdit">
         <property name="calendarPopup">
          <bool>true</bool>
         </property>
        </widget>
       </item>
       <item>
        <widget class="QTimeEdit" name="timeEdit">
         <property name="displayFormat">
          <string>HH:mm</string>
         </property>
        </widget>
       </item>
       <item>
        <spacer name="timeSpacer">
         <property name="orientation">
          <enum>Qt::Horizontal</enum>
         </property>
        </spacer>
       </item>
      </layout>
     </item>

     <!-- 持续时间 -->
     <item row="2" column="0">
      <widget class="QLabel" name="durationLabel">
       <property name="text">
        <string>持续时间:</string>
       </property>
      </widget>
     </item>
     <item row="2" column="1">
      <widget class="QComboBox" name="durationCombo"/>
     </item>

     <!-- 时区 -->
     <item row="3" column="0">
      <widget class="QLabel" name="timezoneLabel">
       <property name="text">
        <string>时区:</string>
       </property>
      </widget>
     </item>
     <item row="3" column="1">
      <widget class="QComboBox" name="timezoneCombo"/>
     </item>
    </layout>
   </item>

   <!-- 分组间距 -->
   <item>
    <spacer name="afterBasicSpacer">
     <property name="orientation">
      <enum>Qt::Vertical</enum>
     </property>
     <property name="sizeHint" stdset="0">
      <size>
       <width>20</width>
       <height>15</height>
      </size>
     </property>
    </spacer>
   </item>

   <!-- 参会人员分组 -->
   <item>
    <widget class="QLabel" name="attendeesGroupLabel">
     <property name="font">
      <font>
       <pointsize>10</pointsize>
       <bold>true</bold>
      </font>
     </property>
     <property name="styleSheet">
      <string notr="true">QLabel {
    background-color: #f5f5f5;
    color: #666666;
    padding: 5px 15px;
    border-radius: 3px;
}</string>
     </property>
     <property name="text">
      <string>━━━ 参会人员 ━━━</string>
     </property>
     <property name="alignment">
      <set>Qt::AlignCenter</set>
     </property>
    </widget>
   </item>

   <!-- 参会人员表单 -->
   <item>
    <layout class="QFormLayout" name="attendeesFormLayout">
     <property name="labelAlignment">
      <set>Qt::AlignRight|Qt::AlignVCenter</set>
     </property>
     <property name="formAlignment">
      <set>Qt::AlignLeading|Qt::AlignTop</set>
     </property>
     <property name="horizontalSpacing">
      <number>10</number>
     </property>
     <property name="verticalSpacing">
      <number>12</number>
     </property>

     <!-- 邀请邮箱 -->
     <item row="0" column="0">
      <widget class="QLabel" name="emailsLabel">
       <property name="text">
        <string>邀请邮箱:</string>
       </property>
      </widget>
     </item>
     <item row="0" column="1">
      <widget class="QLineEdit" name="emailsEdit">
       <property name="placeholderText">
        <string>输入邮箱，用逗号或分号分隔</string>
       </property>
      </widget>
     </item>

     <!-- 会议室 -->
     <item row="1" column="0">
      <widget class="QLabel" name="roomLabel">
       <property name="text">
        <string>会议室:</string>
       </property>
      </widget>
     </item>
     <item row="1" column="1">
      <widget class="QComboBox" name="roomCombo"/>
     </item>
    </layout>
   </item>

   <!-- 分组间距 -->
   <item>
    <spacer name="afterAttendeesSpacer">
     <property name="orientation">
      <enum>Qt::Vertical</enum>
     </property>
     <property name="sizeHint" stdset="0">
      <size>
       <width>20</width>
       <height>15</height>
      </size>
     </property>
    </spacer>
   </item>

   <!-- 可折叠分组占位容器 -->
   <item>
    <widget class="QWidget" name="collapsibleContainer" native="true">
     <layout class="QVBoxLayout" name="collapsibleLayout">
      <property name="spacing">
       <number>10</number>
      </property>
      <property name="leftMargin">
       <number>0</number>
      </property>
      <property name="topMargin">
       <number>0</number>
      </property>
      <property name="rightMargin">
       <number>0</number>
      </property>
      <property name="bottomMargin">
       <number>0</number>
      </property>
     </layout>
    </widget>
   </item>

   <!-- 弹性空间 -->
   <item>
    <spacer name="middleSpacer">
     <property name="orientation">
      <enum>Qt::Vertical</enum>
     </property>
    </spacer>
   </item>

   <!-- 底部按钮 -->
   <item>
    <layout class="QHBoxLayout" name="buttonLayout">
     <property name="spacing">
      <number>15</number>
     </property>
     <item>
      <spacer name="leftButtonSpacer">
       <property name="orientation">
        <enum>Qt::Horizontal</enum>
       </property>
      </spacer>
     </item>
     <item>
      <widget class="QPushButton" name="cancelBtn">
       <property name="minimumSize">
        <size>
         <width>80</width>
         <height>36</height>
        </size>
       </property>
       <property name="text">
        <string>取消</string>
       </property>
      </widget>
     </item>
     <item>
      <widget class="QPushButton" name="saveDraftBtn">
       <property name="minimumSize">
        <size>
         <width>80</width>
         <height>36</height>
        </size>
       </property>
       <property name="text">
        <string>保存草稿</string>
       </property>
      </widget>
     </item>
     <item>
      <widget class="QPushButton" name="bookNowBtn">
       <property name="minimumSize">
        <size>
         <width>80</width>
         <height>36</height>
        </size>
       </property>
       <property name="text">
        <string>立即预定</string>
       </property>
      </widget>
     </item>
     <item>
      <spacer name="rightButtonSpacer">
       <property name="orientation">
        <enum>Qt::Horizontal</enum>
       </property>
      </spacer>
     </item>
    </layout>
   </item>

   <!-- 底部间距 -->
   <item>
    <spacer name="bottomSpacer">
     <property name="orientation">
      <enum>Qt::Vertical</enum>
     </property>
    </spacer>
   </item>

  </layout>
 </widget>
 <resources/>
 <connections/>
 <slots>
  <slot>onCancelClicked()</slot>
  <slot>onSaveDraftClicked()</slot>
  <slot>onBookNowClicked()</slot>
 </slots>
</ui>
```

- [ ] **Step 2: 验证 UI 文件格式**

```bash
# 检查文件是否创建成功
ls -la ui/bookmeeting.ui

# 验证 XML 格式（可选，如果有 xmllint）
# xmllint --noout ui/bookmeeting.ui
```

Expected: 文件存在且大小 > 5KB

- [ ] **Step 3: Commit UI 文件**

```bash
git add ui/bookmeeting.ui
git commit -m "design(bookmeeting): 重写 UI 文件，采用 QFormLayout 布局"
```

---

## Task 3: 修改 src/bookmeeting.h

**Files:**
- 修改: `src/bookmeeting.h`

**变更说明：**
- 移除动态创建的控件指针（现在由 ui 文件管理）
- 保留 CollapsibleSection 指针（仍需代码动态创建）
- 保留信号、槽函数和辅助方法

- [ ] **Step 1: 更新 src/bookmeeting.h**

```cpp
#ifndef BOOKMEETING_H
#define BOOKMEETING_H

#include <QWidget>
#include <QDateTime>
#include <QStringList>

namespace Ui {
class BookMeeting;
}

// 会议预定数据结构
struct MeetingBookingInfo {
    // 基本信息
    QString topic;
    QDateTime startTime;
    int durationMinutes;
    QString timeZone;

    // 参会人员
    QStringList inviteEmails;
    QString roomResource;

    // 会议安全
    bool passwordEnabled;
    QString password;
    bool waitingRoomEnabled;
    int joinPermission;

    // 会议设置
    bool autoMuteOnEntry;
    int screenSharePermission;
    int recordingPermission;

    // 高级选项
    int meetingNumberType;
    QString description;

    MeetingBookingInfo();
};

class CollapsibleSection;

class BookMeeting : public QWidget
{
    Q_OBJECT

public:
    explicit BookMeeting(QWidget *parent = nullptr);
    ~BookMeeting();

    MeetingBookingInfo getBookingInfo() const;
    void setBookingInfo(const MeetingBookingInfo &info);

signals:
    void sigMeetingBooked(const MeetingBookingInfo &info);
    void sigDraftSaved();
    void sigCancelled();
    void sigClosing();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onCancelClicked();
    void onSaveDraftClicked();
    void onBookNowClicked();

    // 表单验证
    bool validateForm();

    // 草稿管理
    void saveDraft();
    void loadDraft();
    bool hasDraft() const;

private:
    void setupUI();
    void setupCollapsibleSections();
    void setupConnections();
    void loadOptions();

    Ui::BookMeeting *ui;

    // 可折叠分组（代码动态创建）
    CollapsibleSection *securitySection_ = nullptr;
    CollapsibleSection *settingsSection_ = nullptr;
    CollapsibleSection *advancedSection_ = nullptr;

    // 标记是否有未保存的修改
    bool hasUnsavedChanges_ = false;
};

#endif // BOOKMEETING_H
```

- [ ] **Step 2: Commit 头文件**

```bash
git add src/bookmeeting.h
git commit -m "refactor(bookmeeting): 简化头文件，移除动态控件指针"
```

---

## Task 4: 重写 src/bookmeeting.cpp

**Files:**
- 重写: `src/bookmeeting.cpp`

**实现要点：**
- 使用 ui 文件中的控件
- 动态创建三个 CollapsibleSection
- 为输入框添加图标
- 设置窗口样式

- [ ] **Step 1: 编写新的 src/bookmeeting.cpp**

```cpp
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

    // 设置窗口样式
    setStyleSheet(R"(
        /* 分组标题样式 */
        QLabel#basicGroupLabel, QLabel#attendeesGroupLabel {
            background-color: #f5f5f5;
            color: #666666;
            padding: 5px 15px;
            border-radius: 3px;
            font-size: 10pt;
            font-weight: bold;
        }

        /* 输入框样式 */
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

        /* 按钮样式 */
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

    // 设置按钮 objectName 用于样式表
    ui->bookNowBtn->setObjectName("bookNowBtn");
    ui->saveDraftBtn->setObjectName("saveDraftBtn");
    ui->cancelBtn->setObjectName("cancelBtn");

    // 设置窗口位置和大小
    if (QScreen *screen = QGuiApplication::primaryScreen()) {
        QRect g = screen->availableGeometry();
        move((g.width() - 460) / 2, (g.height() - 620) / 2);
    }
    resize(460, 620);

    // 为输入框添加图标
    ui->topicEdit->addAction(QIcon(":/assets/user.png"), QLineEdit::LeadingPosition);
    ui->emailsEdit->addAction(QIcon(":/assets/email.png"), QLineEdit::LeadingPosition);

    // 设置日期时间默认值
    ui->dateEdit->setDate(QDate::currentDate());
    ui->timeEdit->setTime(QTime::currentTime().addSecs(1800));
    ui->dateEdit->setMinimumDate(QDate::currentDate());
}

void BookMeeting::setupCollapsibleSections()
{
    QVBoxLayout *collapsibleLayout = qobject_cast<QVBoxLayout*>(ui->collapsibleContainer->layout());
    if (!collapsibleLayout) return;

    // 会议安全分组
    securitySection_ = new CollapsibleSection(tr("会议安全"), this);
    securitySection_->setExpanded(false);

    QWidget *securityContent = new QWidget();
    QFormLayout *securityForm = new QFormLayout(securityContent);
    securityForm->setSpacing(12);
    securityForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // 启用密码
    QCheckBox *passwordCheck = new QCheckBox(tr("启用会议密码"));
    passwordCheck->setObjectName("passwordCheck");
    securityForm->addRow(passwordCheck);

    // 密码输入
    QLineEdit *passwordEdit = new QLineEdit();
    passwordEdit->setObjectName("passwordEdit");
    passwordEdit->setPlaceholderText(tr("4-10位数字"));
    passwordEdit->setEnabled(false);
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->addAction(QIcon(":/assets/lock.png"), QLineEdit::LeadingPosition);
    securityForm->addRow(tr("密码:"), passwordEdit);

    connect(passwordCheck, &QCheckBox::toggled, passwordEdit, &QLineEdit::setEnabled);

    // 等候室
    QCheckBox *waitingRoomCheck = new QCheckBox(tr("启用等候室"));
    waitingRoomCheck->setObjectName("waitingRoomCheck");
    waitingRoomCheck->setChecked(true);
    securityForm->addRow(waitingRoomCheck);

    // 入会权限
    QComboBox *permissionCombo = new QComboBox();
    permissionCombo->setObjectName("permissionCombo");
    permissionCombo->addItem(tr("所有人"), 0);
    permissionCombo->addItem(tr("登录用户"), 1);
    permissionCombo->addItem(tr("仅邀请者"), 2);
    securityForm->addRow(tr("入会权限:"), permissionCombo);

    securitySection_->setContent(securityContent);
    collapsibleLayout->addWidget(securitySection_);

    // 会议设置分组
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

    // 高级选项分组
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

    // 标记修改状态
    connect(ui->topicEdit, &QLineEdit::textChanged, this, [this]() { hasUnsavedChanges_ = true; });
    connect(ui->emailsEdit, &QLineEdit::textChanged, this, [this]() { hasUnsavedChanges_ = true; });
}

void BookMeeting::loadOptions()
{
    // 持续时间选项
    ui->durationCombo->addItem(tr("15 分钟"), 15);
    ui->durationCombo->addItem(tr("30 分钟"), 30);
    ui->durationCombo->addItem(tr("45 分钟"), 45);
    ui->durationCombo->addItem(tr("60 分钟"), 60);
    ui->durationCombo->addItem(tr("90 分钟"), 90);
    ui->durationCombo->addItem(tr("120 分钟"), 120);
    ui->durationCombo->setCurrentIndex(3);

    // 时区选项
    ui->timezoneCombo->addItem(tr("(UTC+08:00) 北京"), "Asia/Shanghai");
    ui->timezoneCombo->addItem(tr("(UTC+09:00) 东京"), "Asia/Tokyo");
    ui->timezoneCombo->addItem(tr("(UTC+00:00) 伦敦"), "Europe/London");
    ui->timezoneCombo->addItem(tr("(UTC-05:00) 纽约"), "America/New_York");

    // 会议室选项
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

    // 基本信息
    info.topic = ui->topicEdit->text();
    info.startTime = QDateTime(ui->dateEdit->date(), ui->timeEdit->time());
    info.durationMinutes = ui->durationCombo->currentData().toInt();
    info.timeZone = ui->timezoneCombo->currentData().toString();

    // 参会人员
    info.inviteEmails = ui->emailsEdit->text().split(QRegularExpression("[,;]"), Qt::SkipEmptyParts);
    info.roomResource = ui->roomCombo->currentData().toString();

    // 会议安全（从 CollapsibleSection 中查找）
    if (QCheckBox *passwordCheck = securitySection_->content()->findChild<QCheckBox*>("passwordCheck"))
        info.passwordEnabled = passwordCheck->isChecked();
    if (QLineEdit *passwordEdit = securitySection_->content()->findChild<QLineEdit*>("passwordEdit"))
        info.password = passwordEdit->text();
    if (QCheckBox *waitingRoomCheck = securitySection_->content()->findChild<QCheckBox*>("waitingRoomCheck"))
        info.waitingRoomEnabled = waitingRoomCheck->isChecked();
    if (QComboBox *permissionCombo = securitySection_->content()->findChild<QComboBox*>("permissionCombo"))
        info.joinPermission = permissionCombo->currentData().toInt();

    // 会议设置
    if (QCheckBox *muteCheck = settingsSection_->content()->findChild<QCheckBox*>("muteCheck"))
        info.autoMuteOnEntry = muteCheck->isChecked();
    if (QComboBox *shareCombo = settingsSection_->content()->findChild<QComboBox*>("shareCombo"))
        info.screenSharePermission = shareCombo->currentData().toInt();
    if (QComboBox *recordCombo = settingsSection_->content()->findChild<QComboBox*>("recordCombo"))
        info.recordingPermission = recordCombo->currentData().toInt();

    // 高级选项
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
```

- [ ] **Step 2: Commit 实现文件**

```bash
git add src/bookmeeting.cpp
git commit -m "feat(bookmeeting): 重写实现，使用 QFormLayout 和图标"
```

---

## Task 5: 构建和测试

**Files:**
- 构建: 整个项目
- 测试: BookMeeting 界面功能

- [ ] **Step 1: 配置项目**

```bash
cmake -B build -S . -DCMAKE_PREFIX_PATH="C:/Qt/6.10.2/msvc2022_64"
```

- [ ] **Step 2: 构建项目**

```bash
cmake --build build --config Debug
```

Expected: 构建成功，无错误

- [ ] **Step 3: 运行测试**

手动测试清单：
- [ ] 窗口尺寸为 460×620
- [ ] 窗口居中显示
- [ ] 基本信息分组可见且展开
- [ ] 参会人员分组可见且展开
- [ ] 会议安全分组可折叠，默认折叠
- [ ] 会议设置分组可折叠，默认折叠
- [ ] 高级选项分组可折叠，默认折叠
- [ ] 会议主题输入框有 user.png 图标
- [ ] 邀请邮箱输入框有 email.png 图标
- [ ] 密码输入框有 lock.png 图标（展开后可见）
- [ ] 描述输入框有 user.png 图标（展开后可见）
- [ ] 下拉框无图标
- [ ] 表单验证正常工作（主题必填、时间必须晚于当前）
- [ ] 立即预定按钮样式正确（蓝色背景）
- [ ] 保存草稿按钮样式正确（白底蓝边）
- [ ] 取消按钮样式正确（白底灰边）
- [ ] 草稿保存/加载正常工作

- [ ] **Step 4: Commit 完成标记**

```bash
git add -A
git commit -m "style(bookmeeting): 界面风格统一为 Login/Register 风格

- 窗口尺寸调整为 460×620
- 使用 QFormLayout 表单布局
- 分组标题使用灰色背景样式
- 基本信息和参会人员始终展开
- 安全/设置/高级选项默认可折叠
- 为输入框添加图标（复用现有资源）
- 统一按钮和输入框样式"
```

---

## 验收标准

| 检查项 | 状态 |
|--------|------|
| 窗口尺寸 460×620 | ☐ |
| 基本信息分组始终展开，灰色标题 | ☐ |
| 参会人员分组始终展开，灰色标题 | ☐ |
| 会议安全默认可折叠 | ☐ |
| 会议设置默认可折叠 | ☐ |
| 高级选项默认可折叠 | ☐ |
| 主题输入框有 user.png 图标 | ☐ |
| 邮箱输入框有 email.png 图标 | ☐ |
| 密码输入框有 lock.png 图标 | ☐ |
| 描述输入框有 user.png 图标 | ☐ |
| 下拉框无图标 | ☐ |
| 表单验证正常工作 | ☐ |
| 草稿功能正常工作 | ☐ |
| 构建无错误 | ☐ |
