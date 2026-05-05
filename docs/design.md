# MeetEx 设计文档

日期: 2026-05-05

## 设计摘要

MeetEx 应在现有可运行 LiveKit 客户端基础上继续演进，不建议推倒重写。推荐保留当前三层结构:

- `src/` 和 `ui/`: Qt Widgets 界面层。
- `meetingengine/`: 会议房间、参会者和 LiveKit 事件桥接。
- `mediaengine/`: 设备采集、播放、渲染、音量分析和媒体 worker。

下一阶段应在 UI 和引擎之间增加轻量控制器/服务边界，让登录、入会、聊天、录制、屏幕共享、设置等功能有明确职责，避免继续把产品逻辑堆到窗口类里。

## 当前架构

```text
Qt Widgets UI
  Login, Home, JoinMeeting, InMeeting, Settings, Profile
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

这条主链路已经适合继续扩展。主要问题是 UI 类承担了过多流程状态，并且若干产品功能缺少模型或服务边界。

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

窗口类不应直接处理后端 token、会议创建或复杂 LiveKit 状态机，应调用控制器或服务并响应结果信号。

### 应用服务

实现功能时建议新增以下轻量服务:

- `AuthService`: 登录、退出、恢复会话、获取用户资料。
- `MeetingDirectoryService`: 快速会议、加入会议校验、预定会议、会议详情和邀请文本。
- `SettingsStore`: 本地设备偏好和入会默认选项持久化。
- `RecordingService`: 通过后端开始/停止录制，并观察录制状态。

这些服务可以先做成普通 C++/Qt 类，不需要引入大型框架。

### 会议会话控制器

`MeetingSessionController` 作为单场会议的所有者。职责包括:

- 保存 `MeetingSessionContext` 中的房间 URL、token、会议号、显示名称和入会选项。
- 通过引擎客户端适配器持有并驱动 `MeetingEngine`。
- 将 UI 命令转换为会议和媒体操作。
- 通过 Qt 信号向 `InMeeting` 暴露会议状态。
- 在会议窗口关闭时统一清理媒体和房间资源。

这样 `InMeeting` 可以专注于显示和交互，不必成为所有会议业务逻辑的容器。

### meetingengine

`meetingengine` 保持为 LiveKit 房间层:

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

加入流程生成 `MeetingJoinRequest`:

```text
meetingNumber
displayName
autoConnectAudio
startCamera
startMicrophone
source = join | quick | scheduled | share-screen
```

请求交给 `MeetingDirectoryService`，成功后返回:

```text
meetingId
roomName
livekitUrl
livekitToken
userIdentity
displayName
permissions
```

只有拿到该结果后才打开会中页。会中页创建会议会话控制器，并将返回结果和入会选项传入。

阶段 1 的客户端实现通过 `MeetingDirectoryService` 屏蔽会议来源。开发环境使用 `DevelopmentMeetingDirectoryService`，由工厂读取 `MEETEX_LIVEKIT_URL`、`MEETEX_LIVEKIT_TOKEN` 和 `MEETEX_DEFAULT_ROOM` 生成入会结果；接入后端时使用 `HttpMeetingDirectoryService` 调用 `/api/meetings/join` 和 `/api/meetings/quick` 获取 LiveKit URL 与 token。成功后，`MeetingSessionContext` 会把目录服务返回结果和入会选项一并传入 `InMeeting` / `MeetingSessionController`，会中页面只消费会话上下文和控制器信号，不直接拼接 token 或决定会议来源。

### 会中媒体状态

会议会话控制器维护本地媒体状态:

```text
roomState: disconnected | connecting | connected | reconnecting | disconnecting
microphoneState: off | starting | on | stopping | failed
cameraState: off | starting | on | stopping | failed
screenShareState: off | starting | on | stopping | failed
recordingState: idle | starting | active | stopping | failed
```

按钮文本和可用性应由状态驱动。点击按钮只请求状态迁移，不能直接把按钮文本当作状态源。

### 参会者模型

每个参会者维护一个模型:

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

`MeetingRoom` 已经能发出大部分原始事件。会议会话控制器负责合并这些事件，然后向 `InMeeting` 输出稳定的参会者状态。

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
