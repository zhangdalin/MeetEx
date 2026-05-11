# MeetEx 设计文档

日期: 2026-05-05

## 设计摘要

MeetEx 应在现有可运行 LiveKit 客户端基础上继续演进，不建议推倒重写。推荐保留当前三层结构:

- `src/` 和 `ui/`: Qt Widgets 界面层。
- `meetingengine/`: 会议会话、会议房间、参会者和 LiveKit 事件桥接。
- `mediaengine/`: 设备采集、播放、渲染、音量分析和媒体 worker。

当前代码已经在 UI 和引擎之间补上了第一层轻量边界，即 `MeetingSession`。下一阶段应继续把入会目录服务、聊天、录制、屏幕共享、设置等功能从窗口类里抽离出来，避免重新把产品逻辑堆回 UI。

## 当前架构

```text
Qt Widgets UI
  Login, Home, JoinMeeting, InMeeting, Settings, Profile
        |
        v
MeetingSession
  保存会话上下文，驱动入会/离会和本地媒体状态
        |
        v
MeetingEngine
  持有 MeetingRoom，提供入会/离会/音视频操作
        |
        v
MeetingRoom
  包装 livekit::Room，接收 LiveKit 回调并发出 Qt 信号
        |
        v
MediaEngine
  MediaMgr 的单例门面
        |
        v
MediaMgr
  Qt 麦克风/摄像头采集、扬声器播放、视频帧缓存、
  播放/渲染线程、音量分析
```

这条主链路已经适合继续扩展。当前主要问题不再是完全缺少会话层，而是会话层只完成了最小实现，`JoinMeeting`、`Home` 和 `InMeeting` 之间仍有若干流程与状态留在 UI 里。

## 当前实现状态

截至当前代码基线:

- `meetingengine/meeting_session.*` 已提供 `MeetingSession`、`MeetingSessionCtx` 和基础入会选项。
- `InMeeting` 已改为持有 `MeetingSession`，不再直接持有 `MeetingEngine`。
- `MeetingSession` 已接管房间连接、离开会议、麦克风/摄像头开关、本地/远端音量查询，以及 `MeetingRoom` 事件转发。
- `MeetingRoom` 已支持从外部注入 LiveKit URL 和 token，日志中 token 采用脱敏输出。
- `JoinMeeting` 和 `Home` 仍未真实构造 `MeetingSessionCtx`，当前默认仍通过开发环境固定 URL 和 token 进入会议。
- `Participant` 状态仍主要由 `InMeeting` 直接组织，尚未抽成独立的参会者模型。

## 推荐模块边界

### UI 层

UI 层只负责展示状态、收集输入和组织控件:

- `Login`: 收集登录信息并展示认证状态。
- `Home`: 提供功能入口和页面路由。
- `JoinMeeting`: 收集会议号、显示名称和入会选项。
- `BookMeeting`: 收集预定会议信息。
- `InMeeting`: 展示会议状态、参会者、工具栏和功能面板。
- `Participant`: 展示单个参会者视频、头像、名称和音量状态。
- `Settings`: 编辑本地偏好。

当前实现里，`InMeeting` 已改为调用 `MeetingSession` 并响应其信号；但 `Home` 和 `JoinMeeting` 还没有把真实入会参数传进会话层，这部分仍需补齐。

### 应用服务

实现功能时建议新增以下轻量服务:

- `AuthService`: 登录、退出、恢复会话、获取用户资料。
- `MeetingService`: 快速会议、加入会议校验、预定会议、会议详情和邀请文本。
- `SettingsStore`: 本地设备偏好和入会默认选项持久化。
- `RecordingService`: 通过后端开始/停止录制，并观察录制状态。

这些服务可以先做成普通 C++/Qt 类，不需要引入大型框架。

### 会议会话

`MeetingSession` 作为单场会议的所有者。职责包括:

- 保存 `MeetingSessionCtx` 中的房间 URL、token、会议号、显示名称和入会选项。
- 通过引擎客户端适配器持有并驱动 `MeetingEngine`。
- 将 UI 命令转换为会议和媒体操作。
- 通过 Qt 信号向 `InMeeting` 暴露会议状态。
- 在会议窗口关闭时统一清理媒体和房间资源。

当前已落地的 `MeetingSession` 已覆盖以下职责:

- 保存 `MeetingSessionCtx` 中的房间 URL、token、会议号、显示名称和入会选项。
- 持有 `MeetingEngine` 并驱动 `start()`、`shutdown()`、`startAudio()`、`stopAudio()`、`startVideo()`、`stopVideo()`。
- 暴露 `roomState`、`microphoneState`、`cameraState` 和会话错误信号。
- 转发参会者加入离开、轨道订阅和退订事件给 UI。

仍待补齐的职责:

- 根据目录服务或后端返回结果构造 `MeetingSessionCtx`，而不是使用开发默认值。
- 把录制、屏幕共享和聊天等会中能力继续并入会话或其子控制器。
- 输出稳定的参会者视图模型，减少 `InMeeting` 直接维护 `Participant` 和轨道 SID 的负担。

### meetingengine

`meetingengine` 保持为 LiveKit 房间层:

- `MeetingSession`: 单场会议会话边界，负责会话上下文、本地媒体状态和 UI 事件入口。
- 连接和断开房间。
- 将 LiveKit 回调桥接为 Qt 信号。
- 发布和取消发布本地媒体轨道。
- 跟踪远端参会者和远端轨道生命周期。
- 暴露连接质量、重连中、重连成功、断开等状态。

该层不应处理登录、预定会议、邀请文案或录制权限等产品概念。

### mediaengine

`mediaengine` 保持为平台媒体层:

- 麦克风采集并写入 LiveKit audio source。
- 摄像头采集并写入 LiveKit video source。
- 后续新增屏幕采集源。
- 远端音频播放和混音。
- 远端视频帧读取和缓存。
- 本地和远端音量分析。
- worker 生命周期和确定性清理。

屏幕共享应作为独立采集路径加入，不应复用或扭曲摄像头采集逻辑。

## 功能设计

### 登录与资料

登录窗口调用 `AuthService`，成功后得到本地会话对象。会话对象包含用户 id、显示名、头像信息和后端访问凭据。LiveKit token 不属于登录凭据，只在创建或加入具体会议时按房间申请。

个人资料页读取本地会话缓存，并可通过 `AuthService` 刷新。

### 加入会议与快速会议

目标加入流程生成 `MeetingJoin`:

```text
meetingNumber
displayName
startCamera
startMicrophone
source = join | quick | scheduled | share-screen
```

请求交给 `MeetingService`，成功后返回:

```text
meetingId
roomName
livekitUrl
livekitToken
userIdentity
displayName
permissions
```

只有拿到该结果后才打开会中页。会中页创建 `MeetingSession`，并将返回结果和入会选项传入。

当前代码与目标状态之间还有一段距离:

- `Home::onQuickMeeting()` 仍直接打开 `InMeeting`。
- `JoinMeeting::onJoinMeeting()` 仍未收集真实会议号和显示名，也未请求后端。
- `InMeeting` 默认通过 `MeetingSessionCtx::developmentDefaults()` 创建会话。

因此，下一步应先把 `JoinMeeting` 和 `Home` 改为显式构造 `MeetingSessionCtx`，再引入目录服务接口。

### 会中媒体状态

目标状态机如下:

```text
roomState: disconnected | connecting | connected | reconnecting | disconnecting
microphoneState: off | starting | on | stopping | failed
cameraState: off | starting | on | stopping | failed
screenShareState: off | starting | on | stopping | failed
recordingState: idle | starting | active | stopping | failed
```

当前代码已经落地的状态只有:

```text
roomState: disconnected | connecting | connected | reconnecting | disconnecting
microphoneState: off | starting | on | stopping | failed
cameraState: off | starting | on | stopping | failed
```

`MeetingSession` 已持有这些状态，但 `InMeeting` 的按钮文本仍在本地更新，没有完全改造成纯状态驱动。录制和屏幕共享状态尚未进入 `MeetingSession`。

### 参会者模型

目标上，每个参会者维护一个模型:

```text
identity
displayName
metadata
audioTrackSid
videoTrackSid
screenShareTrackSid
audioMuted
videoMuted
isSpeaking
audioLevel
connectionQuality
```

`MeetingRoom` 已经能发出大部分原始事件。当前代码中，`MeetingSession` 只负责转发事件，`InMeeting` 仍直接维护 `participants_`、`audioTrackOwners_` 和来宾名生成逻辑。下一步应把这些状态收敛到独立参会者模型，再由 `InMeeting` 只做渲染。

### 聊天

MVP 聊天使用 LiveKit data message 实现房间级文本消息。

推荐流程:

1. 用户在聊天面板输入文本。
2. 聊天控制器校验文本并创建本地 pending 消息。
3. 通过 LiveKit data message 发送。
4. 发送成功后标记为 sent。
5. 收到远端 data message 后解析并追加到聊天模型。

如果后续需要离线历史或多端同步，可以在同一消息模型之上增加后端历史接口。

### 屏幕共享

屏幕共享应新增独立媒体源:

```text
ScreenCaptureSource -> livekit::VideoSource -> LocalVideoTrack
```

发布参数应使用 LiveKit 屏幕共享 source 元数据。会议模型需要区分屏幕共享轨道和摄像头轨道，方便 UI 对共享内容做优先展示。

远端屏幕共享不应覆盖该参会者的摄像头轨道，而应作为同一参会者拥有的第二路视频流。

### 录制

MVP 推荐采用后端控制的 LiveKit Egress 服务端录制。

客户端职责:

- 展示用户是否有录制权限。
- 向后端发送开始/停止录制请求。
- 展示开始中、录制中、停止中和失败状态。
- 通过房间 metadata 或后端状态同步录制状态。

后端职责:

- 校验主持人或录制权限。
- 启动和停止 LiveKit Egress。
- 保存录制文件元数据和回放/下载地址。
- 将录制状态变化同步给客户端。

### 设置

`SettingsStore` 持久化本地偏好:

```text
defaultMicrophoneDeviceId
defaultSpeakerDeviceId
defaultCameraDeviceId
joinWithAudio
joinWithCamera
joinWithMicrophone
language
logLevel
recordingPreference
```

当采集适配层支持指定设备 id 后，媒体层应读取这些设置。支持之前，设置页也可以先保存偏好并展示当前默认设备行为。

## 错误处理

错误信息分两层:

- 用户提示: 简短、可本地化、说明用户能做什么。
- 诊断日志: 记录房间、轨道、后端和设备细节。

重点错误包括:

- 登录失败。
- 会议不存在或已过期。
- LiveKit token 获取失败。
- 房间连接失败。
- 重连中和断开。
- 麦克风或摄像头权限失败。
- 轨道发布或取消发布失败。
- 远端轨道订阅失败。
- 屏幕采集失败。
- 录制开始或停止失败。

完整 token 和密钥不得写入日志。

## 测试策略

手工验证应覆盖:

- 登录成功和失败。
- 加入会议时摄像头/麦克风开关组合。
- 快速会议进入。
- 两端真实音视频会议。
- 远端参会者加入和离开。
- 关闭摄像头后的头像兜底。
- 静音/解除静音和发言状态。
- 网络重连和断开。
- 屏幕共享开始/停止。
- 聊天发送/接收。
- 录制状态变化。

自动化测试可以从以下位置开始:

- 控制器状态迁移单元测试。
- 设置持久化单元测试。
- 聊天消息序列化单元测试。
- 有 LiveKit 测试房间时，补充 `MeetingRoom` 事件到 Qt 信号的集成测试。

## 安全与隐私

- 产品构建不得编译固定 LiveKit token。
- 会议 token 应短时有效，并限制到具体房间和身份。
- 日志应脱敏 token 和后端凭据。
- 屏幕共享时应有明确的本地提示。
- 录制必须有显式权限和会中可见状态。

## 实施注意事项

- 保持 `meetingengine` 和 `mediaengine` 不依赖 UI 控件。
- 不要继续把后端产品逻辑直接加入 `InMeeting`。
- 在扩展窗口逻辑前，优先补小型控制器和服务类。
- 跨线程和跨层状态更新使用 Qt 信号。
- 房间销毁前应明确停止媒体 worker。
