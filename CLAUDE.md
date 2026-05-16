# CLAUDE.md

本文件为 Claude Code (claude.ai/code) 提供在此仓库中工作的指导。

## 构建命令

这是一个使用 CMake 的 Qt/C++ Windows 桌面项目。VS Code 工作空间已预配置 Qt 6.10.2 和 Visual Studio 2022 路径。

### 配置并构建 (Debug)

```powershell
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="D:/Qt/6.10.2/msvc2022_64" -B build -S .
cmake --build build --config Debug --parallel
```

### 仅构建 (在 build 目录下)

```powershell
cd build
cmake --build . --config Debug --parallel
```

### 构建输出

- **可执行文件**: `build/Debug/MeetEx.exe`
- **库文件**: `build/mediaengine/Debug/mediaengine.lib`, `build/meetingengine/Debug/meetingengine.lib`
- **工具**: `build/tools/Debug/avatar_generator.exe`

构建自动执行：
- 从 `thirdparty/livekit/lib/<Debug|Release>/` 复制 LiveKit DLL (`livekit.dll`, `livekit_ffi.dll`)
- 使用 `windeployqt` 部署 Qt 运行时依赖

## 架构

### 三层结构

```
UI 层 (src/, ui/)
    |
    v
MeetingSession (meetingengine/meeting_session.*)
    - 持有 MeetingSessionCtx (URL、token、会议号、显示名称、入会选项)
    - 驱动 MeetingEngine 生命周期
    - 将 UI 命令桥接到媒体操作
    |
    v
MeetingEngine + MeetingRoom (meetingengine/)
    - 包装 livekit::Room
    - 将 LiveKit 回调桥接到 Qt 信号
    - 管理参会者生命周期和轨道订阅
    |
    v
MediaEngine (mediaengine/)
    - MediaEngine: 媒体操作的单例门面
    - MediaMgr: Qt 麦克风/摄像头采集、扬声器播放、视频渲染线程
    - 平台适配器: media_qmic, media_qcam, media_qspk
```

### 关键组件

- **MeetingSession**: 拥有单场会议。UI 启动/加入/离开会议和控制本地媒体的入口点。发出房间状态、媒体状态和参会者变化的 Qt 信号。
- **MeetingRoom**: LiveKit 的 livekit::Room 的轻量包装，带 Qt 信号发射。
- **MeetingParticipant**: 参会者状态模型 (身份、轨道、音频/视频静音、发言状态)。
- **MediaEngine/MediaMgr**: 处理设备采集、音频播放、视频渲染和音量分析。

### 状态机

MeetingSession 暴露以下状态枚举：
- `MeetingSessionRoomState`: disconnected | connecting | connected | reconnecting | disconnecting
- `MeetingSessionMediaState`: off | starting | on | stopping | failed

## 开发环境变量

设置以下变量可在不修改代码的情况下控制 LiveKit 连接：

```powershell
$env:MEETEX_LIVEKIT_URL = "wss://your-livekit-server"
$env:MEETEX_LIVEKIT_TOKEN = "your-token"
$env:MEETEX_DEFAULT_ROOM = "room-name"
$env:MEETEX_API_BASE_URL = "https://your-backend-api"
```

## 项目结构

- **src/**: Qt Widgets UI 类 (Login, Home, JoinMeeting, InMeeting, Settings 等)
- **ui/**: Qt .ui 表单文件
- **meetingengine/**: LiveKit 房间集成、会话管理、参会者模型
- **mediaengine/**: 音频/视频采集、播放、渲染
- **tools/**: 头像生成工具
- **thirdparty/livekit/**: LiveKit C++ SDK 头文件和库
- **i18n/**: Qt 翻译文件
- **res/**: Qt 资源文件和资源

## 日志

- 日志同时写入控制台和工作目录下的 `meetex.log`
- Token 和凭据值在日志中被掩码 (参见 `meetingengine/meeting_security.h`)
- 日志级别可通过 Qt 消息处理器控制

## 安全注意事项

- 切勿将固定的 LiveKit token 编译到发布版本中
- Token 必须是短期的，并限定于特定房间/身份
- 始终在输出前掩码日志中的 token
