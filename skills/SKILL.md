---
name: meetex-patterns
description: Coding patterns extracted from MeetEx Qt/LiveKit video conferencing project
version: 1.1.0
source: local-git-analysis
analyzed_commits: 50
---

# MeetEx 项目编码模式

## 项目概述

MeetEx 是基于 Qt 6 的 Windows 桌面视频会议应用程序，使用 LiveKit C++ SDK 进行实时音视频通信。旨在构建一个完整的企业级视频会议系统。

**技术栈：**
- **UI 框架:** Qt 6 Widgets
- **媒体 SDK:** LiveKit C++ SDK
- **构建工具:** CMake
- **语言标准:** C++17

---

## 提交规范

本项目使用 **中文提交信息** 配合 **Conventional Commits** 前缀：

### 提交前缀

| 前缀 | 用途 | 示例 |
|------|------|------|
| `feat:` | 新功能 | `feat(meeting): 添加屏幕共享支持` |
| `feat(module):` | 模块新功能 | `feat(ui): 添加参会者列表面板` |
| `fix:` | 修复问题 | `fix: 修复视频渲染闪烁问题` |
| `fix(module):` | 模块修复 | `fix(meeting): 修复房间断开时媒体未停止` |
| `refactor:` | 重构 | `refactor(ui): 重命名 ParticipantWidget 为 GLWidget` |
| `refactor(module):` | 模块重构 | `refactor(meeting): 合并 session 逻辑` |
| `chore:` | 杂项/配置 | `chore: 添加 livekit 部署配置` |
| `docs:` | 文档更新 | `docs: 更新 README 和项目文档` |
| `build:` | 构建相关 | `build(windows): 自动部署 Qt 运行时 DLL` |
| `perf:` | 性能优化 | `perf: 缓存成员控件优化音频状态更新` |
| `test:` | 测试相关 | `test: 添加会议会话单元测试` |

### 提交格式

```
<type>(<scope>): <中文描述>

<详细说明（可选）>
```

**示例：**
```
feat(meeting): 添加房间事件回调和优化视频渲染

- 实现 RoomDelegate 所有回调函数
- 添加视频帧缓存机制
- 优化头像兜底显示逻辑

Refs: AUTH-001, MEET-005
```

---

## 代码架构

### 目录结构

```
MeetEx/
├── src/                    # UI 层 - Qt Widgets
│   ├── login.cpp/h         # 登录窗口 (AUTH-001~007)
│   ├── home.cpp/h          # 主菜单/主页 (MEET-001~003)
│   ├── joinmeeting.cpp/h   # 加入会议 (MEET-002)
│   ├── bookmeeting.cpp/h   # 预约会议 (MEET-003)
│   ├── inmeeting.cpp/h     # 会议中窗口 (MEET-005~008, SOC-001~005)
│   ├── settings.cpp/h      # 设置容器 (SET-V/A/M-xxx)
│   ├── participantwidget.* # 参会者视频卡片 (SOC-001)
│   ├── memberwidget.*      # 成员列表项 (SOC-001)
│   ├── glwidget.*          # OpenGL 视频渲染 (MEDIA-006)
│   └── main.cpp            # 入口点
├── ui/                     # Qt Designer 界面文件
│   └── *.ui               # 窗体定义
├── meetingengine/          # 会议引擎层
│   ├── meeting_session.*  # 会议会话管理 (MEET-xxx, MGMT-xxx)
│   ├── meeting_room.*     # LiveKit 房间封装 (NET-001~003)
│   ├── meeting_participant.* # 参会者模型 (SOC-001)
│   └── meeting_def.h      # 常量定义 (LIVEKIT_URL, token)
├── mediaengine/            # 媒体引擎层
│   ├── media_engine.*     # 媒体引擎门面 (MEDIA-001~007)
│   ├── media_mgr.*        # 媒体管理器
│   ├── media_qcam.*       # 摄像头采集 (SET-V-001)
│   ├── media_qmic.*       # 麦克风采集 (SET-A-001)
│   ├── media_qspk.*       # 扬声器播放 (SET-A-002)
│   ├── media_filewav.*    # WAV 文件播放
│   └── media_def.h        # 媒体常量 (AUDIO_*, VIDEO_*)
├── tools/                  # 工具
│   └── avatar_generator.* # 头像生成
├── thirdparty/livekit/     # LiveKit C++ SDK
│   ├── include/livekit/   # SDK 头文件
│   └── lib/               # 库文件 (Debug/Release)
├── res/                    # 资源文件
│   ├── assets/            # 图标、图片
│   └── *.qrc              # Qt 资源定义
├── docs/                   # 文档
│   ├── requirements.md    # 需求规格说明书
│   └── *.md               # 其他文档
└── CMakeLists.txt         # CMake 主配置
```

### 架构分层

```
┌─────────────────────────────────────────────────────────┐
│  UI 层 (src/, ui/)                                       │
│  - 登录、主页、加入/预约会议、会议中、设置等窗口            │
│  - 信号槽连接，响应引擎层事件                              │
├─────────────────────────────────────────────────────────┤
│  会议引擎 (meetingengine/)                               │
│  - MeetingSession: 单场会议上下文 (P0 核心功能)             │
│  - MeetingRoom: LiveKit 房间封装 (WebSocket/RTC)          │
│  - MeetingParticipant: 参会者模型 (身份、轨道、状态)         │
├─────────────────────────────────────────────────────────┤
│  媒体引擎 (mediaengine/)                                 │
│  - MediaEngine: 媒体操作门面 (单例)                       │
│  - MediaMgr: 设备管理 (采集/播放/渲染)                    │
│  - 视频渲染: GLWidget (OpenGL)                           │
└─────────────────────────────────────────────────────────┘
```

---

## 编码规范

### 命名约定

| 类型 | 规范 | 示例 |
|------|------|------|
| 类名 | PascalCase | `MeetingSession`, `MediaEngine`, `InMeeting` |
| 方法 | camelCase | `startAudio()`, `onParticipantJoined()` |
| 成员变量 | snake_case + 下划线后缀 | `context_`, `roomState_`, `localParticipant_` |
| 信号 | `sig` 前缀 | `sigParticipantJoined`, `sigRoomStateChanged` |
| 槽函数 | `on` 前缀 或 `toggle` | `onLogin()`, `toggleAudio()` |
| 文件名 | snake_case | `meeting_session.cpp`, `media_engine.h` |
| 宏/常量 | 全大写下划线 | `AUDIO_SAMPLE_RATE`, `LIVEKIT_URL` |
| 枚举 | PascalCase + 后缀 | `MeetingSessionRoomState`, `TrackKind` |

### 头文件结构

```cpp
#ifndef CLASS_NAME_H
#define CLASS_NAME_H

#include <QtHeaders>
#include "local_headers.h"

// 前置声明
namespace Ui { class FormName; }
class DependencyClass;

/**
 * @brief 类简要说明
 * 
 * 详细说明：功能、职责、使用场景
 * 
 * @author 作者
 * @date 日期
 */
class ClassName : public BaseClass
{
    Q_OBJECT

public:
    explicit ClassName(QWidget *parent = nullptr);
    ~ClassName();

    // Getters
    StateType state() const { return state_; }
    
    // Public methods
    bool startOperation();
    void stopOperation();

signals:
    void sigStateChanged(StateType state);
    void sigError(const QString &message);

public slots:
    void onAction();
    void toggleState();

private:
    // UI
    Ui::FormName *ui;
    
    // State
    StateType state_;
    
    // Dependencies
    std::unique_ptr<DependencyClass> dependency_;
    
    // Private methods
    void setupUi();
    void setupConnections();
    void setState(StateType state);
};

#endif // CLASS_NAME_H
```

### 状态机模式

会议和媒体状态使用枚举状态机：

```cpp
// meeting_session.h
enum class MeetingSessionRoomState {
    Disconnected,    // 已断开
    Connecting,      // 连接中
    Connected,       // 已连接
    Reconnecting,    // 重连中
    Disconnecting    // 断开中
};

enum class MeetingSessionMediaState {
    Off,             // 关闭
    Starting,        // 启动中
    On,              // 开启
    Stopping,        // 停止中
    Failed           // 失败
};

enum class TrackKind {
    UNKNOWN = 0,
    AUDIO = 1,
    VIDEO = 2
};
```

状态转换需发出信号通知 UI：
```cpp
void MeetingSession::setRoomState(MeetingSessionRoomState state) {
    if (roomState_ != state) {
        roomState_ = state;
        emit sigRoomStateChanged(state);
        qInfo() << "Room state changed to:" << static_cast<int>(state);
    }
}
```

---

## 数据模型

### 会议会话上下文

```cpp
// meeting_def.h
struct MeetingSessionJoinOptions {
    bool startCamera = true;      // MEET-006
    bool startMicrophone = true;  // MEET-005
    bool startScreenShare = false; // MEET-007
    bool startRecording = false;   // MEET-008
};

struct MeetingSessionCtx {
    QString livekitUrl;           // NET-001
    QString livekitToken;         // SEC-001 (需脱敏)
    QString meetingNumber;        // MEET-002 (9位数字)
    QString displayName;          // SOC-001
    MeetingSessionJoinOptions joinOptions;
    
    bool isValid() const;
    static MeetingSessionCtx defaults(); // 开发默认值
};
```

### 参会者模型

```cpp
// meeting_participant.h
class MeetingParticipant {
public:
    QString identity;             // 唯一标识
    QString displayName;          // 显示名称
    
    // 轨道 SID
    QString audioTrackSid;
    QString videoTrackSid;
    QString screenShareTrackSid;
    
    // 状态
    bool audioMuted = false;
    bool videoMuted = false;
    bool isSpeaking = false;
    bool isLocal = false;
    
    // 连接质量
    int connectionQuality = 0;    // QOS-001
    
    // 音量 (0-100)
    int audioLevel = 0;
};
```

### 聊天消息模型

```cpp
// 预留：用于 SOC-002 实时聊天
struct ChatMessage {
    QString id;                   // 消息ID
    QString senderId;             // 发送者ID
    QString senderName;           // 发送者名称
    QString content;              // 内容
    QDateTime timestamp;          // 时间戳
    bool isLocal = false;         // 是否本地发送
    bool delivered = false;       // 是否已送达
};
```

---

## 功能模块实现指南

### 用户认证模块 (AUTH-xxx)

**相关文件:**
- `src/login.cpp/h` - 登录窗口
- `src/myprofile.cpp/h` - 个人资料
- `meetingengine/meeting_def.h` - Token 配置

**实现要点:**
```cpp
// 登录成功后保存会话
class AuthService {
public:
    bool login(const QString &account, const QString &password);
    bool loginWithPhone(const QString &phone, const QString &code);
    void logout();
    bool isLoggedIn() const;
    QString getAccessToken() const;  // AUTH-006
    
signals:
    void sigLoginSuccess(const UserProfile &profile);
    void sigLoginFailed(const QString &error);
};
```

**验收标准对应:**
- AUTH-001: 账号密码登录表单验证
- AUTH-006: Token 自动刷新机制
- AUTH-007: 清理本地缓存和会话

### 会议模块 (MEET-xxx)

**核心流程:**

```
1. 快速会议 (MEET-001)
   Home::onQuickMeeting() -> MeetingService::createQuickMeeting() 
   -> 获取会议号/Token -> 创建 MeetingSession -> 进入 InMeeting

2. 加入会议 (MEET-002)
   JoinMeeting::onJoin() -> 验证会议号 -> 获取 Token 
   -> 创建 MeetingSessionCtx -> 进入 InMeeting

3. 会议控制 (MEET-005~008)
   InMeeting -> MeetingSession -> MediaEngine -> LiveKit
```

**静音/视频切换实现:**
```cpp
// InMeeting 中调用
void InMeeting::toggleAudio() {
    if (meetingSession_->microphoneState() == MeetingSessionMediaState::On) {
        meetingSession_->stopAudio();  // MEET-005
    } else {
        meetingSession_->startAudio();
    }
}

// MeetingSession 处理状态机
void MeetingSession::setMicrophoneState(MeetingSessionMediaState state) {
    microphoneState_ = state;
    emit sigMicrophoneStateChanged(state);
}
```

### 社交模块 (SOC-xxx)

**参会者管理 (SOC-001):**
```cpp
// InMeeting 中维护参会者列表
QHash<QString, ParticipantWidget*> participantWidgets_;  // identity -> widget

void InMeeting::onParticipantJoined(const QString &participantId) {
    auto widget = new ParticipantWidget(this);
    widget->setParticipantInfo(participant);
    participantWidgets_[participantId] = widget;
    updateLayout();
}
```

**实时聊天 (SOC-002):**
```cpp
// 使用 LiveKit Data Message
void InMeeting::sendMsg() {
    QString text = ui->chatInput->text();
    // 发送 data message
    meetingSession_->sendChatMessage(text);
}

// 接收消息
void InMeeting::onChatMessageReceived(const ChatMessage &msg) {
    ui->chatPanel->addMessage(msg);
}
```

### 设置模块 (SET-xxx)

**设备选择实现:**
```cpp
// settingaudio.cpp - SET-A-001, SET-A-002
void SettingAudio::loadDevices() {
    auto micDevices = MediaEngine::instance().getMicrophoneDevices();
    ui->micCombo->addItems(micDevices);
    
    auto speakerDevices = MediaEngine::instance().getSpeakerDevices();
    ui->speakerCombo->addItems(speakerDevices);
}

// 保存到 QSettings
void SettingAudio::saveSettings() {
    QSettings settings("MeetEx", "Config");
    settings.setValue("audio/microphone", ui->micCombo->currentText());
    settings.setValue("audio/speaker", ui->speakerCombo->currentText());
}
```

---

## 关键设计模式

### 1. LiveKit 事件桥接

将 LiveKit C++ SDK 回调转换为 Qt 信号：

```cpp
// meeting_room.h
class MeetingRoom : public QObject, public livekit::RoomDelegate {
    Q_OBJECT
public:
    // RoomDelegate 回调
    void onParticipantConnected(
        livekit::Room &room, 
        const livekit::ParticipantConnectedEvent &ev) override;
    
    void onTrackSubscribed(
        livekit::Room &room,
        const livekit::TrackSubscribedEvent &ev) override;
    
    // ... 其他回调

signals:
    void sigParticipantJoined(const QString &participantId);
    void sigParticipantLeft(const QString &participantId);
    void sigTrackSubscribed(const QString &trackSid, int trackKind);
    void sigTrackUnsubscribed(const QString &trackSid, int trackKind);
    void sigConnectionStateChanged(int state);
};
```

### 2. 媒体引擎单例

```cpp
// media_engine.h
class MediaEngine {
public:
    static MediaEngine &instance() {
        static MediaEngine instance;  // C++11 线程安全
        return instance;
    }
    
    // 禁止拷贝
    MediaEngine(const MediaEngine &) = delete;
    MediaEngine &operator=(const MediaEngine &) = delete;
    
    // 初始化
    bool init();
    bool fini();
    
    // 本地媒体控制
    bool startLocalAudio(livekit::LocalParticipant* participant, 
                        std::string& sid);
    void stopLocalAudio(livekit::LocalParticipant* participant, 
                       const std::string& sid);
    bool startLocalVideo(livekit::LocalParticipant* participant, 
                        std::string& sid);
    void stopLocalVideo(livekit::LocalParticipant* participant, 
                       const std::string& sid);
    
    // 远程媒体
    bool startAudioPlay(const std::shared_ptr<livekit::AudioStream> &stream,
                       const std::string& track_sid);
    bool startVideoRender(const std::shared_ptr<livekit::VideoStream> &stream,
                         const std::string &track_sid);
    
    // 音量检测
    AudioLevelInfo localAudioLevel() const;
    bool isLocalAudioSpeaking() const;
    
private:
    MediaEngine() = default;
    ~MediaEngine() = default;
    
    std::shared_ptr<MediaMgr> media_mgr_;
};
```

### 3. 分层架构分离

```cpp
// UI 层 - 只响应信号
class InMeeting : public QWidget {
    void toggleAudio() {
        // 调用会话层，不直接操作媒体
        meetingSession_->toggleAudio();
    }
    
    void onMicrophoneStateChanged(MeetingSessionMediaState state) {
        // 更新 UI 按钮状态
        updateAudioButton(state);
    }
};

// 会话层 - 业务逻辑
class MeetingSession : public QObject {
    void toggleAudio() {
        if (microphoneState_ == MeetingSessionMediaState::On) {
            stopAudio();
        } else {
            startAudio();
        }
    }
    
    void stopAudio() {
        setMicrophoneState(MeetingSessionMediaState::Stopping);
        // 调用媒体引擎
        MediaEngine::instance().stopLocalAudio(...);
        setMicrophoneState(MeetingSessionMediaState::Off);
    }
};

// 媒体引擎层 - 底层操作
class MediaEngine {
    void stopLocalAudio(...) {
        // 停止采集、释放资源
        media_mgr_->stopMic();
    }
};
```

---

## 开发工作流

### 添加新 UI 窗口

1. **创建 `.ui` 文件** 到 `ui/` 目录
2. **根据需求设计布局** 和控件, 并设置属性 (如无边框、固定大小等)
3. **为控件添加背景图和样式** (可选), 使用 `res/assets/` 中的图标
4. **创建 `.h` 和 `.cpp`** 到 `src/` 目录
5. **在 `CMakeLists.txt` 中添加:**
   ```cmake
   set(PROJECT_HEADERS src/newwindow.h ...)
   set(PROJECT_SOURCES src/newwindow.cpp ...)
   set(PROJECT_FORMS ui/newwindow.ui ...)
   ```
6. **重新运行 CMake 配置**
7. **在父窗口中添加打开逻辑**

### 实现新功能 (以屏幕共享为例)

**MEET-007 屏幕共享实现步骤:**

1. **UI 层** (`src/inmeeting.cpp`):
   - 添加共享按钮
   - 连接 `toggleShare()` 槽
   - 响应 `sigScreenShareStateChanged` 信号

2. **会话层** (`meetingengine/meeting_session.cpp`):
   ```cpp
   void MeetingSession::toggleShare() {
       if (screenShareState_ == MeetingSessionMediaState::On) {
           stopShare();
       } else {
           startShare();
       }
   }
   
   void MeetingSession::startShare() {
       setScreenShareState(MeetingSessionMediaState::Starting);
       bool ok = MediaEngine::instance().startShareLocalScreen(...);
       setScreenShareState(ok ? MeetingSessionMediaState::On 
                              : MeetingSessionMediaState::Failed);
   }
   ```

3. **媒体引擎层** (`mediaengine/media_engine.cpp`):
   ```cpp
   bool MediaEngine::startShareLocalScreen(...) {
       // 实现屏幕采集
       // 创建 LiveKit VideoSource
       // 发布轨道
   }
   ```

### 构建流程

```bash
# 配置（首次或 CMakeLists.txt 变更后）
cmake -B build -S . -DCMAKE_PREFIX_PATH="C:/Qt/6.10.2/msvc2022_64"

# 构建 Debug
cmake --build build --config Debug --parallel

# 构建 Release
cmake --build build --config Release --parallel

# 运行
./build/Debug/MeetEx.exe

# 清理
cmake --build build --target clean
```

---

## 性能优化指南

### UI 渲染优化

```cpp
// 缓存控件，避免重复查找
void InMeeting::updateParticipantWidgets() {
    // 使用缓存的 widgets，不要每次都 findChild
    for (auto widget : cachedOrderedWidgets_) {
        widget->update();
    }
}

// 降低更新频率
void InMeeting::onTimer() {
    audioUpdateCounter_++;
    if (audioUpdateCounter_ % 5 == 0) {  // 每5帧更新一次
        updateAudioStatusPanel();
    }
}
```

### 视频渲染优化

```cpp
// GLWidget 中使用双缓冲
class GLWidget : public QOpenGLWidget {
    void paintGL() override {
        // 快速复制帧缓冲区
        frameMutex_.lock();
        // 渲染
        frameMutex_.unlock();
    }
};
```

---

## 测试策略

### 单元测试

```cpp
// test_meeting_session.cpp
void TestMeetingSession::testStateTransition() {
    MeetingSession session(ctx);
    QCOMPARE(session.roomState(), MeetingSessionRoomState::Disconnected);
    
    QSignalSpy spy(&session, &MeetingSession::sigRoomStateChanged);
    session.start();
    
    QVERIFY(spy.wait(5000));
    QCOMPARE(session.roomState(), MeetingSessionRoomState::Connected);
}
```

### 集成测试

- **会议流程:** 创建 -> 加入 -> 离开
- **媒体流程:** 开启摄像头/麦克风 -> 关闭 -> 重新开启
- **重连测试:** 断网 -> 等待重连 -> 恢复

### 性能测试

| 指标 | 测试方法 | 目标值 |
|------|----------|--------|
| 启动时间 | 从点击到主界面 | < 3 秒 |
| 入会延迟 | 点击加入 -> 进入会议 | < 5 秒 |
| 内存占用 | 50人参会时 | < 500 MB |
| CPU 占用 | 空闲时 | < 5% |

---

## 安全规范

### 敏感数据处理

```cpp
// Token 脱敏示例
QString maskToken(const QString &token) {
    if (token.length() <= 10) return token;
    return token.left(5) + "..." + token.right(5);
}

// 日志中脱敏
qInfo() << "Connecting to room with token:" << maskToken(token);
```

### 加密传输

- WebSocket: wss:// (TLS)
- WebRTC: DTLS-SRTP
- 可选 E2EE: 端到端加密 (SEC-002)

---

## 注意事项

### 线程安全

- **UI 操作** 必须在主线程执行
- **LiveKit 回调** 在内部线程，需通过信号/槽转到主线程
- **媒体采集** 在独立工作线程，使用互斥锁保护共享数据

```cpp
// 安全地更新 UI
void onLiveKitCallback() {
    QMetaObject::invokeMethod(this, [this]() {
        // UI 操作
        updateLabel();
    }, Qt::QueuedConnection);
}
```

### 资源管理

- **离开会议时** 必须停止所有媒体 worker
- **使用 QPointer** 管理可能销毁的 UI 对象
- **视频渲染** 使用 GLWidget，确保 OpenGL 上下文正确释放

### 日志规范

- 使用 `qInfo()`, `qDebug()`, `qWarning()` 输出日志
- 敏感信息（Token、密码）必须脱敏
- 日志写入 `meetex.log` 文件
- 定期清理日志文件

---

## 功能实现优先级

### P0 - MVP 核心 (立即实现)

| 功能 | 文件 | 说明 |
|------|------|------|
| AUTH-001 | login.cpp | 账号密码登录 |
| AUTH-006 | login.cpp | 会话保持 |
| MEET-001 | home.cpp | 快速会议 |
| MEET-002 | joinmeeting.cpp | 加入会议 |
| MEET-005 | inmeeting.cpp | 静音/解除静音 |
| MEET-006 | inmeeting.cpp | 开启/关闭视频 |
| SOC-001 | participantwidget.cpp | 参会者管理 |
| MEDIA-001~006 | mediaengine/ | 音视频引擎 |

### P1 - 重要功能 (第二阶段)

| 功能 | 文件 | 说明 |
|------|------|------|
| MEET-003 | bookmeeting.cpp | 预约会议 |
| MEET-007 | inmeeting.cpp | 屏幕共享 |
| MEET-008 | inmeeting.cpp | 会议录制 |
| SOC-002 | inmeeting.cpp | 实时聊天 |
| SET-xxx | settings.cpp | 设置模块 |

### P2/P3 - 增值/未来功能

- 云端录制 (REC-002)
- 白板 (COL-004)
- 实时字幕 (COL-005)
- 插件机制 (EXT-001)

---

## 相关文件

| 文件 | 说明 |
|------|------|
| `CLAUDE.md` | 项目开发指南 |
| `docs/requirements.md` | 需求规格说明书 (80+ 功能点) |
| `CMakeLists.txt` | 构建设置 |
| `.claude/settings.json` | Claude Code 配置 |

---

## 附录

### 术语表

| 术语 | 说明 |
|------|------|
| LiveKit | 开源实时音视频基础设施 |
| WebRTC | 网页实时通信技术 |
| E2EE | 端到端加密 |
| QoS | 服务质量 |
| MVP | 最小可行产品 |
| SDP | 会话描述协议 |
| ICE | 交互式连接建立 |
| TURN | 中继服务器 |
| SID | 会话标识符 |

### 参考文档

- [Qt 6 文档](https://doc.qt.io/qt-6/)
- [LiveKit C++ SDK](https://github.com/livekit/client-sdk-cpp)
- [WebRTC 标准](https://webrtc.org/)
