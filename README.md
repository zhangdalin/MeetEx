# MeetEx

MeetEx 是一款面向 Windows 桌面端的视频会议客户端，基于 Qt/C++ 和 LiveKit 构建，致力于提供稳定、清晰、低延迟的在线会议体验，覆盖日常沟通、远程协作、在线培训、客户会议和团队例会等典型场景。

MeetEx 让用户能够快速创建和加入会议，在会中完成成员管理、屏幕共享、消息沟通和录制控制，并在复杂网络和设备环境下获得可预期的音视频体验。客户端侧重实时音视频质量、清晰的协作流程、可靠的设备控制和可诊断的运行状态。

## 产品定位

MeetEx 定位为轻量但完整的企业级视频会议客户端:

- 面向 Windows 桌面用户，提供稳定的会议入口和会中体验。
- 基于 LiveKit 实时通信能力，支持多人音视频互动。
- 以 Qt Widgets 构建原生桌面界面，适合企业内部工具、私有化部署和定制化会议场景。
- 通过清晰的会议、媒体和 UI 模块边界，支持后续扩展会议管理、录制、企业通讯录和诊断能力。

## 核心产品能力

MeetEx 围绕视频会议的完整工作流提供以下能力:

- 账号登录、会话恢复、个人资料展示和退出登录。
- 快速会议、会议号加入、预定会议和邀请链接复制。
- 入会前设备偏好选择，包括麦克风、摄像头和自动连接音频。
- 会中多人音视频宫格、头像兜底、发言人高亮和音量状态显示。
- 麦克风静音/解除静音、摄像头开启/关闭和本地媒体状态同步。
- 成员列表、成员状态、连接质量和基础成员操作。
- 房间级文本聊天、消息发送状态和重连后的消息处理。
- 屏幕或窗口共享、远端共享内容优先展示和共享状态提示。
- 服务端录制控制、录制权限提示和录制状态同步。
- 音频、视频、录制、语言、日志和默认入会选项设置。
- 网络重连、设备异常、权限失败、入会失败和轨道订阅失败的用户提示。
- 日志记录、诊断导出和敏感信息脱敏。

## 文档

- [需求文档](docs/requirements.md): 产品功能需求和验收标准。
- [设计文档](docs/design.md): 客户端架构、模块边界和核心数据流。
- [路线图](docs/roadmap.md): 产品演进和实施阶段。

## 技术栈

- C++17
- Qt 5 或 Qt 6: Widgets、OpenGLWidgets、Multimedia、LinguistTools
- CMake 3.16+
- LiveKit C++ SDK 和 FFI 二进制文件
- Windows 桌面端

## 构建

示例 Debug 构建命令:

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
```

Windows 下，顶层 `CMakeLists.txt` 会在构建后从 `thirdparty/livekit/lib/<Debug|Release>` 拷贝 LiveKit 运行时 DLL 到可执行文件目录。如果可以找到 `windeployqt`，也会自动部署 Qt 运行时依赖。

## 运行说明

- 客户端运行日志默认写入进程工作目录下的 `meetex.log`。
- 开发环境可通过 `MEETEX_LIVEKIT_URL`、`MEETEX_LIVEKIT_TOKEN` 和 `MEETEX_DEFAULT_ROOM` 提供会议连接信息，便于不改代码连接指定 LiveKit 房间。
- 接入后端后，可通过 `MEETEX_API_BASE_URL` 指向会议服务，客户端会通过会议服务获取 LiveKit URL 和 token。
- 生产环境应由后端生成短时、按房间和用户身份授权的 LiveKit token。
- 客户端日志和诊断信息应避免输出完整 token、密钥或其他敏感凭据。
- 媒体层支持在设备缺失时使用调试兜底音视频源，便于开发和演示。

## 项目结构

```text
.
|-- CMakeLists.txt
|-- README.md
|-- docs/
|   |-- requirements.md          # 产品功能需求
|   |-- design.md                # 客户端架构和模块设计
|   |-- roadmap.md               # 产品演进路线
|   `-- superpowers/plans/       # 分阶段实施计划文件
|-- i18n/
|   `-- MeetEx_zh_CN.ts          # 中文翻译资源
|-- mediaengine/
|   |-- media_engine.*           # 会议媒体能力门面
|   |-- media_mgr.*              # 采集、播放、渲染、音量分析和线程管理
|   |-- media_qmic.*             # Qt 麦克风采集适配
|   |-- media_qcam.*             # Qt 摄像头采集适配
|   |-- media_qspk.*             # Qt 扬声器播放适配
|   |-- media_filewav.*          # WAV 测试/兜底音频源
|   |-- media_util.*             # LiveKit 媒体辅助函数
|   `-- media_def.h              # 本地媒体参数常量
|-- meetingengine/
|   |-- meeting_engine.*         # UI 使用的会议能力门面
|   |-- meeting_room.*           # LiveKit Room 包装和 Qt 信号桥接
|   |-- local_user.*             # 本地参会者媒体操作
|   |-- remote_user.*            # 远端参会者包装
|   |-- meeting_security.h       # token 日志脱敏辅助
|   `-- meeting_def.h            # 历史/底层会议常量，运行时连接信息由应用层提供
|-- res/
|   |-- home.qrc
|   `-- assets/                  # 登录和主页图片资源
|-- src/
|   |-- main.cpp                 # QApplication、日志和翻译初始化
|   |-- app/                     # 入会模型、目录服务、HTTP 边界、会话控制器和引擎适配器
|   |-- login.*                  # 登录窗口
|   |-- home.*                   # 主页和顶层窗口路由
|   |-- joinmeeting.*            # 加入会议窗口
|   |-- bookmeeting.*            # 预定会议窗口
|   |-- sharescreen.*            # 共享屏幕窗口
|   |-- inmeeting.*              # 会中 UI 和 LiveKit 事件绑定
|   |-- participant.*            # 参会者卡片和音量浮层
|   |-- glwidget.*               # OpenGL 视频/头像渲染
|   |-- settings.*               # 设置窗口外壳
|   |-- settingaudio.*           # 音频设置页
|   |-- settingvideo.*           # 视频设置页
|   |-- settingrecording.*       # 录制设置页
|   |-- settingcommon.*          # 通用设置页
|   `-- myprofile.*              # 个人资料页
|-- style/                       # 样式资源
|-- tests/
|   |-- app/                     # 应用层入会模型、目录服务和会话控制器测试
|   `-- meetingengine/           # 会议引擎安全辅助测试
|-- thirdparty/livekit/
|   |-- include/livekit/         # LiveKit 头文件
|   `-- lib/Debug|Release        # LiveKit 导入库和运行库
|-- tools/
|   |-- avatar_generator.*       # 头像生成工具
|   `-- avatar_generator_main.cpp
`-- ui/
    |-- login.ui
    |-- home.ui
    |-- joinmeeting.ui
    |-- inmeeting.ui
    |-- bookmeeting.ui
    |-- sharescreen.ui
    |-- settings.ui
    |-- settingaudio.ui
    |-- settingvideo.ui
    |-- settingrecording.ui
    |-- settingcommon.ui
    |-- myprofile.ui
    `-- participant.ui
```
