# JoinMeeting 界面设计方案

## 设计概述

重新设计 JoinMeeting（加入会议）界面，使其与 BookMeeting（预定会议）界面风格保持一致，提供更专业、统一的用户体验。

## 窗口规格

- **尺寸**: 420 × 550 像素
- **最小尺寸**: 380 × 500 像素
- **布局**: 垂直滚动布局（QScrollArea + QVBoxLayout）
- **窗口标题**: "加入会议"

## 界面布局

```
┌─────────────────────────────────────────────────────────┐
│                      加入会议 (标题)                      │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  ━━━ 会议信息 ━━━                                       │
│  会议号:      [输入框              ▼] [使用个人会议号]   │
│  会议密码:    [密码输入框            ]                   │
│                                                         │
│  ━━━ 参会者信息 ━━━                                     │
│  您的名称:    [输入框                    ]               │
│                                                         │
│  ━━━ 设备预览 [展开 ▼] ━━━                              │
│  ┌──────────────────────┬──────────────────────┐       │
│  │                      │                      │       │
│  │    摄像头画面        │    音频电平指示       │       │
│  │    预览区域          │    ▓▓▓▓▓▒▒▒▒         │       │
│  │                      │                      │       │
│  │                      │    [测试扬声器]       │       │
│  ├──────────────────────┼──────────────────────┤       │
│  │ 当前设备: [下拉框▼]  │ 当前设备: [下拉框▼]   │       │
│  │           [视频设置] │           [音频设置]  │       │
│  └──────────────────────┴──────────────────────┘       │
│                                                         │
│  ━━━ 入会设置 ━━━                                       │
│           [✓] 自动连接音频                              │
│           [✓] 入会开启摄像头                            │
│           [✓] 入会开启麦克风                            │
│                                                         │
├─────────────────────────────────────────────────────────┤
│        [取消]  [加入会议]                               │
└─────────────────────────────────────────────────────────┘
```

## 分组详情

### 1. 会议信息分组

**会议号输入**
- 控件: QLineEdit + QComboBox（可编辑下拉框）
- 占位文本: "请输入会议号"
- 功能:
  - 支持下拉显示最近加入的会议历史记录
  - 输入时实时匹配历史记录
  - 限制输入为数字（9-11位）

**使用个人会议号按钮**
- 控件: QPushButton
- 样式: 小号次要按钮
- 功能: 点击后自动填入用户的个人会议号

**会议密码输入**
- 控件: QLineEdit
- 样式: 默认隐藏，需要时展开显示
- 属性: setEchoMode(QLineEdit::Password)
- 占位文本: "请输入会议密码"
- 展开动画: QPropertyAnimation 实现高度从 0 到 35px 的平滑过渡

### 2. 参会者信息分组

**您的名称**
- 控件: QLineEdit
- 占位文本: "请输入您的显示名称"
- 默认值: 用户上次使用的名称或系统用户名

### 3. 设备预览分组（可折叠）

使用 CollapsibleSection 组件实现，默认折叠状态。

**展开后布局 - 左侧面板（视频区域）**
- 摄像头预览画面
  - 控件: QVideoWidget 或自定义渲染组件
  - 尺寸: 宽度约 50%，高度自适应 4:3 比例
- 当前设备标签: "当前设备:"
- 摄像头下拉框: QComboBox，显示当前摄像头名称
- 视频设置按钮: QPushButton，点击跳转到 SettingVideo 界面

**展开后布局 - 右侧面板（音频区域）**
- 音频电平指示
  - 控件: QProgressBar 或自定义绘制组件
  - 显示实时麦克风音量（0-100%）
- 测试扬声器按钮: QPushButton，点击播放测试音
- 当前设备标签: "当前设备:"
- 麦克风下拉框: QComboBox，显示当前麦克风名称
- 音频设置按钮: QPushButton，点击跳转到 SettingAudio 界面

**分隔线**
- 1px 灰色竖线 (#e0e0e0)
- 贯穿整个设备预览区域，将视频和音频完全分隔

### 4. 入会设置分组

三个 QCheckBox 垂直排列:
- [✓] 自动连接音频（默认勾选）
- [✓] 入会开启摄像头（默认勾选）
- [✓] 入会开启麦克风（默认勾选）

## 按钮区域

**取消按钮**
- 样式:
  ```css
  background-color: #ffffff;
  border: 1px solid #e0e0e0;
  border-radius: 6px;
  padding: 8px 20px;
  color: #333333;
  font-size: 13px;
  min-height: 36px;
  min-width: 80px;
  ```
- 悬停: background-color: #f5f5f5

**加入会议按钮**
- 样式:
  ```css
  background-color: #0078d4;
  border: none;
  border-radius: 6px;
  padding: 8px 20px;
  color: white;
  font-size: 13px;
  font-weight: bold;
  min-height: 36px;
  min-width: 80px;
  ```
- 悬停: background-color: #006cbd

## 样式规范（与 BookMeeting 保持一致）

| 元素 | 样式 |
|------|------|
| 分组标签 | 背景 #f5f5f5，文字 #666666，字体 10pt 加粗，圆角 3px，padding 5px 15px，居中对齐 |
| 表单标签 | 右对齐，垂直居中，horizontalSpacing: 10px |
| 输入框 | 占位文本提示，focus 状态蓝色边框 #0078d4 |
| 下拉框 | 与输入框相同样式，支持下拉箭头 |
| 复选框 | 左对齐排列，check 状态蓝色 |
| 主按钮 | 背景 #0078d4，白色文字，圆角 6px |
| 次按钮 | 白色背景，#e0e0e0 边框，圆角 6px |
| 分隔线 | 1px #e0e0e0 |
| 间距 | 分组间: 15px，表单项间: 12px，边距: 25px 左右，20px 顶部 |

## 交互细节

### 会议号输入
1. 用户输入时实时匹配历史记录
2. 下拉列表显示最近 5 条历史记录（会议号 + 时间）
3. 点击历史记录自动填入会议号

### 密码框动态显示
1. 输入会议号后，向后端查询会议信息
2. 如会议需要密码，密码输入框平滑展开（动画 200ms）
3. 如会议不需要密码，保持隐藏

### 设备预览
1. 点击"设备预览"标题栏展开/折叠
2. 展开时延迟 200ms 启动摄像头预览（避免界面卡顿）
3. 折叠时停止摄像头预览以释放资源
4. 音频电平指示实时更新（100ms 间隔）

### 设备切换
1. 下拉框列出所有可用设备
2. 选择新设备后立即切换并更新预览
3. 点击"设置"按钮打开对应设置界面（SettingVideo/SettingAudio）

### 表单验证
1. 会议号不能为空
2. 如需要密码，密码不能为空
3. 您的名称不能为空（默认填入上次使用名称）
4. 验证失败时输入框边框变红并显示提示

## 数据结构

```cpp
struct JoinMeetingInfo {
    QString meetingNumber;      // 会议号
    QString password;           // 会议密码（可选）
    QString displayName;        // 显示名称
    bool autoConnectAudio;      // 自动连接音频
    bool enableCamera;          // 入会开启摄像头
    bool enableMicrophone;      // 入会开启麦克风
    QString selectedCamera;     // 选中的摄像头设备
    QString selectedMicrophone; // 选中的麦克风设备
};
```

## 信号定义

```cpp
signals:
    void sigJoinMeeting(const JoinMeetingInfo &info);  // 加入会议
    void sigCancelled();                                 // 取消
    void sigClosing();                                   // 窗口关闭
```

## 历史记录存储

使用 QSettings 存储最近加入的会议:
- 路径: `HKEY_CURRENT_USER\Software\MeetEx\MeetingHistory`
- 格式: 列表存储，最多 10 条
- 内容: 会议号、加入时间

## 实现文件

| 文件 | 说明 |
|------|------|
| `ui/joinmeeting.ui` | Qt Designer 界面文件 |
| `src/joinmeeting.h` | 头文件，声明类和信号 |
| `src/joinmeeting.cpp` | 实现文件 |

## 依赖组件

- CollapsibleSection（可折叠分组组件，复用 BookMeeting 的实现）
- MediaEngine（获取可用设备列表、启动预览）
- SettingVideo（视频设置界面）
- SettingAudio（音频设置界面）

## 参考文件

- `ui/bookmeeting.ui` - 样式参考
- `src/bookmeeting.h/cpp` - 代码结构参考
