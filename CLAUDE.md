# CLAUDE.md

本文档为 Claude Code (claude.ai/code) 提供在本仓库中工作时的指导。

## 项目概述

MeetEx 是一款基于 Qt 的 Windows 桌面视频会议应用程序，使用 LiveKit 进行实时音视频通信。该应用提供加入/创建会议、屏幕共享、录制、会中聊天和参会者管理等功能。

## 构建系统

**构建工具：** CMake（最低版本 3.16）
**编程语言：** C++17
**UI 框架：** Qt 6（兼容 Qt 5）
**构建目录：** `build/`

### 常用构建命令

```bash
# 配置 Qt（确保 Qt 在 PATH 中或设置 CMAKE_PREFIX_PATH）
cmake -B build -S . -DCMAKE_PREFIX_PATH="C:/Qt/6.10.2/msvc2022_64"

# 构建 Debug 版本
cmake --build build --config Debug

# 构建 Release 版本
cmake --build build --config Release

# 并行构建
cmake --build build --config Debug --parallel

# 清理构建
cmake --build build --target clean

# 完全重新构建
rm -rf build && cmake -B build -S . && cmake --build build --config Debug
```

### Qt 部署

在 Windows 上，构建完成后会自动运行 `windeployqt` 来复制 Qt 运行时依赖。输出的可执行文件和 DLL 位于 `build/Debug/` 或 `build/Release/`。

## 架构

代码库采用三层架构：

```
┌─────────────────────────────────────────────────────────┐
│  UI 层 (src/, ui/)                                       │
│  - Qt 控件：登录、主页、会议中、设置等                     │
├─────────────────────────────────────────────────────────┤
│  会议引擎 (meetingengine/)                               │
│  - MeetingSession：单场会议的上下文和状态                 │
│  - MeetingParticipant：参会者模型                        │
│  - LiveKit 房间事件桥接 (RoomDelegate)                  │
├─────────────────────────────────────────────────────────┤
│  媒体引擎 (mediaengine/)                                 │
│  - MediaEngine：媒体操作的单例门面                        │
│  - MediaMgr：设备采集、播放、渲染                        │
│  - 集成 LiveKit C++ SDK                                 │
└─────────────────────────────────────────────────────────┘
```

### 核心组件

**MeetingSession** (`meetingengine/meeting_session.h`)
- 单场会议的核心类
- 管理房间连接状态、媒体状态（麦克风/摄像头/屏幕共享/录制）
- 将 LiveKit 事件桥接到 Qt 信号
- 使用 `MeetingSessionCtx` 创建，包含 LiveKit URL、令牌、会议号和入会选项

**MediaEngine** (`mediaengine/media_engine.h`)
- 提供媒体操作的单例
- 处理本地音视频采集和发布
- 管理远端音频播放和视频渲染
- 提供音量/语音检测

**LiveKit 集成** (`thirdparty/livekit/`)
- 使用 LiveKit C++ SDK (livekit.dll, livekit_ffi.dll)
- 通过 WebSocket/WebRTC 连接房间
- Windows 构建时自动复制 DLL

## 项目结构

```
MeetEx/
├── CMakeLists.txt              # 主构建配置
├── src/                        # UI 源文件
│   ├── main.cpp               # 入口点、日志设置
│   ├── login.cpp/h            # 登录窗口
│   ├── home.cpp/h             # 主菜单/主页
│   ├── inmeeting.cpp/h        # 会议中窗口
│   ├── joinmeeting.cpp/h      # 加入会议对话框
│   ├── bookmeeting.cpp/h      # 预约会议对话框
│   ├── settings.cpp/h         # 设置容器
│   ├── participantwidget.cpp/h # 参会者视频卡片
│   └── ...
├── ui/                        # Qt Designer .ui 文件
├── meetingengine/             # 会议会话层
│   ├── meeting_session.cpp/h
│   ├── meeting_participant.cpp/h
│   └── meeting_def.h          # 常量、令牌配置
├── mediaengine/               # 媒体采集/播放层
│   ├── media_engine.cpp/h
│   ├── media_mgr.cpp/h
│   ├── media_qcam.cpp/h       # 摄像头 (Qt Multimedia)
│   ├── media_qmic.cpp/h       # 麦克风
│   ├── media_qspk.cpp/h       # 扬声器
│   └── media_def.h            # 音视频常量
├── tools/                     # 工具
│   └── avatar_generator.cpp/h # 头像图片生成
├── thirdparty/livekit/        # LiveKit C++ SDK
│   ├── include/livekit/       # 头文件
│   └── lib/{Debug,Release}/   # livekit.dll, livekit_ffi.dll
├── res/                       # 资源（图标、qrc）
├── i18n/                      # 翻译文件
└── docs/                      # 设计文档、需求文档
```

## 开发工作流

### 运行应用程序

```bash
# 构建并运行
cmake --build build --config Debug
./build/Debug/MeetEx.exe
```

日志写入工作目录下的 `meetex.log` 文件。

### LiveKit 测试设置

本地开发时，项目包含 `livekit-srv/meetexrapid.sh`，用于：
1. 使用 FFmpeg 从示例视频启动流媒体
2. 使用 `lk` CLI 将模拟参会者加入 LiveKit 房间

需要安装 LiveKit CLI (`lk`) 和 FFmpeg。

### 配置

LiveKit 连接设置位于 `meetingengine/meeting_def.h`：
- `LIVEKIT_URL`: 连接到 LiveKit 服务器的 WebSocket URL
- `LIVEKIT_TOKEN`: 开发访问令牌
- `LIVEKIT_E2EE_KEY`: 端到端加密密钥（可选）

**安全注意：** 产品版本不得嵌入固定令牌。应用应从后端服务获取短期有效令牌。

## 编码规范

- **类名：** PascalCase（例如：`MeetingSession`, `MediaEngine`）
- **方法名：** camelCase（例如：`startAudio()`, `onParticipantJoined()`）
- **成员变量：** snake_case 加下划线后缀（例如：`context_`, `roomState_`）
- **信号：** `sig` 前缀（例如：`sigParticipantJoined`, `sigRoomStateChanged`）
- **槽函数：** `on` 前缀（例如：`onLogin()`, `toggleAudio()`）
- **文件名：** 与类名匹配的 snake_case

## 状态机

`MeetingSession` 中定义的关键状态枚举：

```cpp
MeetingSessionRoomState: Disconnected -> Connecting -> Connected -> Reconnecting/Disconnecting
MeetingSessionMediaState: Off -> Starting -> On <-> Stopping -> Failed
```

媒体状态包括：麦克风、摄像头、屏幕共享、录制。

## 线程模型

- UI 线程：所有 QWidget 操作、MeetingSession 信号
- LiveKit 内部线程：房间事件（回调桥接到 Qt 信号）
- 媒体工作线程：音频采集/播放、视频渲染（通过 MediaMgr）

跨线程更新使用 Qt 的队列信号连接。

## 添加新功能

实现新功能时：

1. **UI 添加：** 在 `ui/` 创建/更新 .ui 文件，在 `src/` 创建对应的 .cpp/.h 文件
2. **会议逻辑：** 扩展 `MeetingSession` 以实现会议范围的功能
3. **媒体功能：** 添加到 `MediaEngine`/`MediaMgr` 以实现采集/播放
4. **状态管理：** 使用现有的状态机模式和信号
5. **常量：** 添加到相应的 `*_def.h` 文件

## 常见问题

- **找不到 windeployqt：** 确保 Qt 的 `bin/` 目录在 PATH 中或 CMAKE_PREFIX_PATH 设置正确
- **LiveKit DLL 缺失：** 构建系统应自动复制；验证 `thirdparty/livekit/lib/` 是否有 Debug/Release 子目录
- **UI 更改未生效：** 删除 `build/MeetEx_autogen/` 强制重新运行 UIC/MOC
- **日志文件被锁定：** 删除 `meetex.log` 前关闭运行中的实例
