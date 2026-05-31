# Book Meeting 界面实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 实现 Book Meeting 预定会议界面，采用可折叠分组布局，600x500 独立窗口，包含 5 个分组（基本信息、参会人员、会议安全、会议设置、高级选项），风格与 home.ui 保持一致。

**Architecture:** 基于现有 BookMeeting 类扩展，实现可折叠分组控件类 `CollapsibleSection`，每个分组包含相关表单字段。使用 Qt Designer 设计 UI 布局，C++ 实现交互逻辑和数据验证。

**Tech Stack:** Qt 6 (兼容 Qt 5), C++17, CMake

---

## 文件结构

| 文件 | 用途 | 操作 |
|------|------|------|
| `ui/bookmeeting.ui` | Qt Designer UI 文件 | 完全重写 |
| `src/bookmeeting.h` | BookMeeting 类头文件 | 大幅扩展 |
| `src/bookmeeting.cpp` | BookMeeting 实现 | 大幅扩展 |
| `src/collapsiblesection.h` | 可折叠分组控件头文件 | 新建 |
| `src/collapsiblesection.cpp` | 可折叠分组控件实现 | 新建 |

---

## Task 1: 创建可折叠分组控件 CollapsibleSection

**Files:**
- Create: `src/collapsiblesection.h`
- Create: `src/collapsiblesection.cpp`

### 类定义

```cpp
class CollapsibleSection : public QWidget {
    Q_OBJECT
public:
    explicit CollapsibleSection(const QString &title, QWidget *parent = nullptr);
    void setContent(QWidget *content);
    void setExpanded(bool expanded);
    bool isExpanded() const;
    void setSummary(const QString &summary);

signals:
    void expansionChanged(bool expanded);

private:
    QToolButton *toggleButton_;
    QLabel *titleLabel_;
    QLabel *summaryLabel_;
    QWidget *contentContainer_;
    QWidget *contentWidget_;
    bool expanded_;
};
```

- [ ] **Step 1.1: 编写 collapsiblesection.h 头文件**

```cpp
#ifndef COLLAPSIBLESECTION_H
#define COLLAPSIBLESECTION_H

#include <QWidget>
#include <QToolButton>
#include <QLabel>

class CollapsibleSection : public QWidget {
    Q_OBJECT

public:
    explicit CollapsibleSection(const QString &title, QWidget *parent = nullptr);
    ~CollapsibleSection();

    void setContent(QWidget *content);
    QWidget* content() const;
    void setExpanded(bool expanded);
    bool isExpanded() const;
    void setSummary(const QString &summary);

signals:
    void expansionChanged(bool expanded);

private slots:
    void onToggleClicked();

private:
    void updateToggleIcon();

    QToolButton *toggleButton_;
    QLabel *titleLabel_;
    QLabel *summaryLabel_;
    QWidget *contentContainer_;
    QWidget *contentWidget_;
    bool expanded_;
};

#endif // COLLAPSIBLESECTION_H
```

- [ ] **Step 1.2: 编写 collapsiblesection.cpp 实现**

```cpp
#include "collapsiblesection.h"
#include <QHBoxLayout>
#include <QVBoxLayout>

CollapsibleSection::CollapsibleSection(const QString &title, QWidget *parent)
    : QWidget(parent), expanded_(true)
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

    connect(header, &QWidget::mousePressEvent, [this]() {
        onToggleClicked();
    });

    updateToggleIcon();
}

CollapsibleSection::~CollapsibleSection() = default;

void CollapsibleSection::setContent(QWidget *content)
{
    if (contentWidget_) delete contentWidget_;
    contentWidget_ = content;
    if (contentWidget_) {
        QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(contentContainer_->layout());
        if (layout) layout->addWidget(contentWidget_);
    }
}

QWidget* CollapsibleSection::content() const { return contentWidget_; }

void CollapsibleSection::setExpanded(bool expanded)
{
    if (expanded_ == expanded) return;
    expanded_ = expanded;
    contentContainer_->setVisible(expanded_);
    summaryLabel_->setVisible(!expanded_ && !summaryLabel_->text().isEmpty());
    updateToggleIcon();
    emit expansionChanged(expanded_);
}

bool CollapsibleSection::isExpanded() const { return expanded_; }

void CollapsibleSection::setSummary(const QString &summary)
{
    summaryLabel_->setText(summary);
    summaryLabel_->setVisible(!expanded_ && !summary.isEmpty());
}

void CollapsibleSection::onToggleClicked() { setExpanded(!expanded_); }

void CollapsibleSection::updateToggleIcon()
{
    toggleButton_->setArrowType(expanded_ ? Qt::DownArrow : Qt::RightArrow);
}
```

- [ ] **Step 1.3: 更新 CMakeLists.txt**

添加新文件到 CMakeLists.txt 的 HEADERS 和 SOURCES 列表。

- [ ] **Step 1.4: 提交**

```bash
git add src/collapsiblesection.h src/collapsiblesection.cpp CMakeLists.txt
git commit -m "feat(bookmeeting): 添加可折叠分组控件 CollapsibleSection"
```

---

## Task 2: 更新 bookmeeting.h 头文件

**Files:**
- Modify: `src/bookmeeting.h`

- [ ] **Step 2.1: 更新头文件**

```cpp
#ifndef BOOKMEETING_H
#define BOOKMEETING_H

#include <QWidget>
#include <QDateTime>

namespace Ui { class BookMeeting; }

struct MeetingBookingInfo {
    QString topic;
    QDateTime startTime;
    int durationMinutes;
    QString timeZone;
    QStringList inviteEmails;
    bool passwordEnabled;
    QString password;
    bool waitingRoomEnabled;
    int joinPermission;
    bool autoMuteOnEntry;
    int screenSharePermission;
    int recordingPermission;
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

signals:
    void sigMeetingBooked(const MeetingBookingInfo &info);
    void sigDraftSaved();
    void sigCancelled();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onCancelClicked();
    void onSaveDraftClicked();
    void onBookNowClicked();
    bool validateForm();
    void saveDraft();
    void loadDraft();
    void updateSectionSummaries();

private:
    void setupUI();
    void setupSections();
    void setupConnections();

    Ui::BookMeeting *ui;
    CollapsibleSection *basicInfoSection_;
    CollapsibleSection *attendeesSection_;
    CollapsibleSection *securitySection_;
    CollapsibleSection *settingsSection_;
    CollapsibleSection *advancedSection_;
    bool hasUnsavedChanges_;
};

#endif // BOOKMEETING_H
```

- [ ] **Step 2.2: 提交**

```bash
git add src/bookmeeting.h
git commit -m "feat(bookmeeting): 更新头文件，添加数据结构和方法声明"
```

---

## Task 3: 重新设计 bookmeeting.ui

**Files:**
- Modify: `ui/bookmeeting.ui`

- [ ] **Step 3.1: 编写新的 UI 文件**

```xml
<?xml version="1.0" encoding="UTF-8"?>
<ui version="4.0">
 <class>BookMeeting</class>
 <widget class="QWidget" name="BookMeeting">
  <property name="geometry">
   <rect><x>0</x><y>0</y><width>600</width><height>500</height></rect>
  </property>
  <property name="minimumSize">
   <size><width>500</width><height>400</height></size>
  </property>
  <property name="windowTitle"><string>预定会议</string></property>
  <property name="styleSheet">
   <string notr="true">QWidget { background-color: #ffffff; }
QLineEdit, QComboBox { border: 1px solid #e0e0e0; border-radius: 4px; padding: 6px; }
QPushButton#bookNowBtn { background-color: #0078d4; color: white; border-radius: 6px; padding: 8px 20px; }</string>
  </property>
  <layout class="QVBoxLayout" name="mainLayout">
   <property name="contentsMargins"><number>20</number></property>
   <item>
    <widget class="QLabel" name="titleLabel">
     <property name="styleSheet"><string>font-size: 18px; font-weight: bold;</string></property>
     <property name="text"><string>预定会议</string></property>
    </widget>
   </item>
   <item>
    <widget class="QScrollArea" name="scrollArea">
     <property name="widgetResizable"><bool>true</bool></property>
     <property name="frameShape"><enum>QFrame::NoFrame</enum></property>
     <widget class="QWidget" name="scrollContent">
      <layout class="QVBoxLayout" name="scrollLayout"/>
     </widget>
    </widget>
   </item>
   <item>
    <layout class="QHBoxLayout" name="buttonLayout">
     <item><spacer name="leftSpacer"/></item>
     <item><widget class="QPushButton" name="cancelBtn"><property name="text"><string>取消</string></property></widget></item>
     <item><widget class="QPushButton" name="saveDraftBtn"><property name="text"><string>保存草稿</string></property></widget></item>
     <item><widget class="QPushButton" name="bookNowBtn"><property name="text"><string>立即预定</string></property></widget></item>
     <item><spacer name="rightSpacer"/></item>
    </layout>
   </item>
  </layout>
 </widget>
</ui>
```

- [ ] **Step 3.2: 提交**

```bash
git add ui/bookmeeting.ui
git commit -m "feat(bookmeeting): 重新设计 UI 布局，600x500 窗口"
```

---

## Task 4: 实现 BookMeeting 构造函数和基础功能

**Files:**
- Modify: `src/bookmeeting.cpp`

- [ ] **Step 4.1-4.5: 实现基础功能**

实现构造函数、setupUI、setupSections（基本信息分组）、setupConnections、closeEvent。

- [ ] **Step 4.6: 提交**

```bash
git add src/bookmeeting.cpp
git commit -m "feat(bookmeeting): 实现基础构造函数和基本信息分组"
```

---

## Task 5: 实现按钮功能和数据管理

**Files:**
- Modify: `src/bookmeeting.cpp`

- [ ] **Step 5.1-5.4: 实现槽函数**

实现 onCancelClicked、onSaveDraftClicked、onBookNowClicked、validateForm、saveDraft、loadDraft。

- [ ] **Step 5.5: 提交**

```bash
git add src/bookmeeting.cpp
git commit -m "feat(bookmeeting): 实现按钮功能和草稿管理"
```

---

## Task 6: 构建和测试

- [ ] **Step 6.1: 构建项目**

```bash
cmake -B build -S . -DCMAKE_PREFIX_PATH="C:/Qt/6.10.2/msvc2022_64"
cmake --build build --config Debug
```

- [ ] **Step 6.2: 运行测试**

```bash
./build/Debug/MeetEx.exe
```

测试步骤：
1. 登录进入 Home 界面
2. 点击"预定会议"按钮
3. 验证窗口尺寸 600x500
4. 测试展开/折叠功能
5. 测试表单验证
6. 测试草稿保存和恢复

- [ ] **Step 6.3: 提交**

```bash
git commit -m "test(bookmeeting): 完成构建和功能测试"
```

---

## 执行选项

**计划已完成并保存到 `docs/superpowers/plans/2025-05-31-book-meeting-impl.md`**

两个执行选项：

1. **Subagent-Driven (推荐)** - 为每个任务派遣新子代理，任务间审查，快速迭代
2. **Inline Execution** - 在此会话中使用 executing-plans 顺序执行任务，批量执行带检查点

**选择哪个方式？**
