---
name: verify-build
description: "验证编译(verify-build)"
disable-model-invocation: true
---

该技能使用 `.vscode/settings.json` 中定义的 CMake 配置和 Visual Studio 生成器，验证工作区构建是否正常。

## 功能

- 读取 `.vscode/settings.json` 中的配置：
  - `cmake.cmakePath`
  - `cmake.generator`
  - `cmake.generatorPlatform`
  - `cmake.configureSettings.CMAKE_PREFIX_PATH`
  - `cmake.configureSettings.CMAKE_INSTALL_PREFIX`
  - `cmake.buildDirectory`
- 使用这些配置执行 CMake 清理配置步骤。
- 对配置的构建目录执行 Debug 构建。
- 捕获构建输出并提取编译器错误信息。
- 报告构建是否成功，以及失败时对应的源文件和错误信息。

## 工作流程

1. 确认工作区根目录并定位 `.vscode/settings.json`。
2. 解析配置并使用其中的 `cmakePath`、生成器、生成器平台和 CMake 配置项。
3. 使用工作区构建目录值运行 CMake 配置：
   - `cmakePath -S . -B <buildDirectory> -G "<generator>" -A <generatorPlatform> -DCMAKE_PREFIX_PATH="..." -DCMAKE_INSTALL_PREFIX="..."`
4. 构建项目：
   - `cmakePath --build <buildDirectory> --config Debug`
5. 如果构建失败，提取构建日志中的错误并返回失败源文件与消息。
6. 如果构建成功，则报告生成的可执行文件或库位置。

## 质量检查

- `.vscode/settings.json` 存在且可读取
- 构建目录创建成功且 CMake 配置成功
- 构建命令运行无错误
- 明确提取并返回任何编译错误

## 示例提示

- “验证编译”
- “运行 verify-build 技能，使用 .vscode CMake 配置配置并构建项目。”
- “验证 CMake/Visual Studio 构建并列出所有剩余的编译错误。”
- “使用工作区 verify-build 技能，报告 `MeetEx` 是否可以成功构建。”
