# Book Meeting 界面设计文档

**日期：** 2025-05-31  
**状态：** 待实现  
**相关文件：** ui/bookmeeting.ui, src/bookmeeting.h, src/bookmeeting.cpp, ui/home.ui

---

## 1. 概述

新建一个**独立窗口**的预定会议界面，通过 home.ui 上的 `bookMeetingBtn` 按钮触发打开。

### 1.1 设计目标
- 提供完整的会议预定功能
- 支持基础信息和扩展字段（可折叠）
- 保持与 home.ui 一致的设计风格
- 良好的用户体验，减少认知负荷

---

## 2. 视觉设计规范

### 2.1 色彩系统
继承 home.ui 的设计风格：

| 用途 | 颜色值 |
|------|--------|
| 主色调（品牌色） | `#0078d4` |
| 背景色 | `#ffffff` |
| 分组区域背景 | `#f5f5f5` 或 `#fafafa` |
| 边框色 | `#e0e0e0` |
| 悬停背景 | `#f0f7ff` |
| 文字主色 | `#333333` |
| 文字次色 | `#666666` |

### 2.2 按钮样式
```css
/* 主按钮（立即预定） */
QPushButton {
    background-color: #0078d4;
    color: white;
    border-radius: 6px;
    padding: 8px 20px;
    font-weight: bold;
}
QPushButton:hover {
    background-color: #006cbd;
}

/* 次按钮（保存草稿、取消） */
QPushButton {
    background-color: #ffffff;
    border: 1px solid #e0e0e0;
    border-radius: 6px;
    padding: 8px 20px;
}
QPushButton:hover {
    background-color: #f5f5f5;
}
```

### 2.3 字体规范
| 元素 | 大小 | 粗细 |
|------|------|------|
| 窗口标题 | 16px | bold |
| 分组标题 | 14px | bold |
| 表单标签 | 13px | normal |
| 输入文字 | 13px | normal |
| 按钮文字 | 13px | bold |

---

## 3. 窗口规格

| 属性 | 值 |
|------|-----|
| 尺寸 | 600×500 |
| 标题 | "预定会议" |
| 窗口类型 | 独立窗口（非模态），可自由移动 |
| 最小尺寸 | 500×400 |
| 最大尺寸 | 800×700（可选，防止过度拉伸） |

---

## 4. 布局结构

采用**可折叠分组（Collapsible Sections）**布局：

```
┌────────────────────────────────────┐
│  预定会议                    [×]   │  ← 标题栏
├────────────────────────────────────┤
│                                    │
│  ▼ 基本信息                        │  ← 展开状态
│  ┌────────────────────────────┐   │
│  │ 会议主题: [____________]   │   │
│  │ 开始时间: [日期][时间]     │   │
│  │ 持续时间: [下拉选择]        │   │
│  │ 时区:    [下拉选择]         │   │
│  └────────────────────────────┘   │
│                                    │
│  ▼ 参会人员                        │  ← 展开状态
│  ┌────────────────────────────┐   │
│  │ 邀请:    [邮箱输入____][+]  │   │
│  │          [已邀请列表...]    │   │
│  │ 会议室:  [下拉选择]         │   │
│  └────────────────────────────┘   │
│                                    │
│  ▶ 会议安全                        │  ← 折叠状态
│                                    │
│  ▶ 会议设置                        │
│                                    │
│  ▶ 高级选项                        │
│                                    │
├────────────────────────────────────┤
│      [取消] [保存草稿] [立即预定]  │  ← 底部按钮，居中对齐
└────────────────────────────────────┘
```

---

## 5. 分组详情

### 5.1 基本信息（默认展开）

| 字段 | 类型 | 必填 | 默认值 | 说明 |
|------|------|------|--------|------|
| 会议主题 | QLineEdit | 是 | 空 | 最多 100 字符 |
| 开始日期 | QDateEdit | 是 | 当前日期 | 日期选择器 |
| 开始时间 | QTimeEdit | 是 | 当前时间+30分钟 | 时间选择器，15分钟间隔 |
| 持续时间 | QComboBox | 是 | 60分钟 | 选项：15/30/45/60/90/120分钟 |
| 时区 | QComboBox | 否 | 系统时区 | 常用时区列表 |

### 5.2 参会人员（默认展开）

| 字段 | 类型 | 必填 | 默认值 | 说明 |
|------|------|------|--------|------|
| 邀请邮箱 | 自定义标签输入 | 否 | 空 | 支持多邮箱，逗号/分号/回车分隔 |
| 已邀请列表 | QListWidget | - | 空 | 显示已添加的邮箱，可删除 |
| 会议室 | QComboBox | 否 | 空 | 可选的物理会议室资源 |

### 5.3 会议安全（默认折叠）

| 字段 | 类型 | 必填 | 默认值 | 说明 |
|------|------|------|--------|------|
| 启用密码 | QCheckBox | 否 | 未勾选 | 勾选后显示密码输入框 |
| 会议密码 | QLineEdit | 否 | 空 | 数字密码，4-10位 |
| 启用等候室 | QCheckBox | 否 | 勾选 | 参会者需主持人允许才能入会 |
| 入会权限 | QComboBox | 否 | 所有人 | 选项：所有人/登录用户/仅邀请者 |

### 5.4 会议设置（默认折叠）

| 字段 | 类型 | 必填 | 默认值 | 说明 |
|------|------|------|--------|------|
| 入会时自动静音 | QCheckBox | 否 | 勾选 | 所有参会者入会时静音 |
| 屏幕共享权限 | QComboBox | 否 | 所有人 | 选项：所有人/仅主持人 |
| 允许录制 | QComboBox | 否 | 仅主持人 | 选项：仅主持人/所有人 |

### 5.5 高级选项（默认折叠）

| 字段 | 类型 | 必填 | 默认值 | 说明 |
|------|------|------|--------|------|
| 会议号类型 | QComboBox | 否 | 自动生成 | 选项：使用个人会议号/自动生成 |
| 添加到日历 | QGroupBox | 否 | 未勾选 | 复选框：Outlook/Google/系统日历 |
| 会议描述 | QTextEdit | 否 | 空 | 多行文本，最多 500 字符 |
| 附件 | 自定义 | 否 | 空 | 支持上传会议相关文档 |

---

## 6. 分组头部控件

每个分组头部是一个可点击的控件，包含：
- **展开/折叠图标**：`▼` 展开状态，`▶` 折叠状态
- **分组标题**：如"基本信息"
- **可选摘要**：折叠时显示已填写的关键信息摘要

**分组头部样式：**
```css
QWidget#sectionHeader {
    background-color: #f5f5f5;
    border-radius: 6px;
    padding: 10px 15px;
}
QWidget#sectionHeader:hover {
    background-color: #e8e8e8;
}
```

---

## 7. 底部按钮区域

**布局：** 水平居中，按钮间距 15px

| 按钮 | 类型 | 行为 |
|------|------|------|
| 取消 | 次按钮 | 关闭窗口，不保存任何信息 |
| 保存草稿 | 次按钮 | 保存当前填写内容到本地配置，关闭窗口 |
| 立即预定 | 主按钮 | 验证必填字段，创建会议，返回会议信息，关闭窗口 |

**按钮顺序（从左到右）：** [取消] [保存草稿] [立即预定]

---

## 8. 交互行为

### 8.1 窗口打开
1. 用户点击 home.ui 的 `bookMeetingBtn`
2. 打开 BookMeeting 窗口（600×500）
3. 基础信息组自动填充默认值
4. 窗口显示在屏幕中央或鼠标附近

### 8.2 表单验证
- 点击"立即预定"时验证必填字段
- 会议主题不能为空
- 开始时间必须晚于当前时间
- 邀请邮箱格式校验
- 验证失败时显示错误提示（红色边框或 tooltip）

### 8.3 草稿保存
- 保存到 QSettings 或本地 JSON 文件
- 下次打开时自动恢复
- 成功预定后清除草稿

### 8.4 关闭事件
- 如果有未保存的修改，提示"是否保存草稿？"
- 否则直接关闭

---

## 9. 数据结构

```cpp
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
    int joinPermission;  // 0=所有人, 1=登录用户, 2=仅邀请者

    // 会议设置
    bool autoMuteOnEntry;
    int screenSharePermission;  // 0=所有人, 1=仅主持人
    int recordingPermission;    // 0=仅主持人, 1=所有人

    // 高级选项
    int meetingNumberType;      // 0=自动生成, 1=个人会议号
    bool addToOutlook;
    bool addToGoogle;
    bool addToSystemCalendar;
    QString description;
    QStringList attachments;
};
```

---

## 10. 信号与槽

### 10.1 BookMeeting 类信号
```cpp
signals:
    void sigMeetingBooked(const MeetingBookingInfo &info);
    void sigDraftSaved();
    void sigCancelled();
```

### 10.2 Home 类槽函数
```cpp
private slots:
    void onBookMeeting();           // 打开预定窗口
    void onMeetingBooked(const MeetingBookingInfo &info);  // 处理预定成功
```

---

## 11. 依赖关系

- 需要创建自定义的**分组折叠控件**（可复用 QWidget + QToolButton）
- 需要自定义**邮箱标签输入控件**（类似收件人输入框）
- 可选：日历集成需要平台特定实现

---

## 12. 验收标准

- [ ] 窗口以 600×500 尺寸打开，可自由移动
- [ ] 包含全部 5 个分组，可折叠/展开
- [ ] 基本信息和参会人员组默认展开
- [ ] 其余组默认折叠
- [ ] 底部三个按钮居中对齐
- [ ] 表单验证正常工作
- [ ] 草稿保存和恢复功能正常
- [ ] 风格与 Login/Register 保持一致
