# 注册页面 UI 重新设计实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 重新设计注册页面 UI，实现分组布局、内嵌验证码按钮、头像预览等功能

**Architecture:** 基于 Qt Widgets 框架，通过修改 .ui 文件重构布局，配合样式表实现视觉效果。保持现有信号/槽连接不变，仅调整 UI 层次结构。

**Tech Stack:** Qt 6 (兼容 Qt 5), CMake, C++17

---

## 文件结构映射

| 文件 | 职责 | 变更类型 |
|------|------|----------|
| `ui/register.ui` | Qt Designer UI 定义文件 | 重写布局结构 |
| `src/register.h` | 注册窗口头文件 | 添加 avatarPreviewLabel 成员 |
| `src/register.cpp` | 注册窗口实现 | 添加头像预览加载逻辑 |

---

## Task 1: 重构 UI 文件布局

**Files:**
- Modify: `ui/register.ui` (完全重写)

**背景:** 当前 UI 是扁平结构，需要重构为分组布局

- [ ] **Step 1: 备份原文件**

```bash
cp ui/register.ui ui/register.ui.bak
```

- [ ] **Step 2: 编写新 UI 文件**

完整的 UI 文件内容（见设计文档中的 XML 代码）

- [ ] **Step 3: 提交 UI 文件变更**

```bash
git add ui/register.ui
git commit -m "feat(ui): redesign register page with grouped layout

- Change window size to 420x600
- Add three groups: account info, contact, profile
- Inline send verification code button
- Add avatar preview section
- Apply consistent styling with design spec"
```

---

## Task 2: 更新头文件添加头像预览成员

**Files:**
- Modify: `src/register.h`

- [ ] **Step 1: 添加 QLabel 前向声明**

- [ ] **Step 2: 添加 avatarPreviewLabel 指针成员**

- [ ] **Step 3: 提交头文件变更**

---

## Task 3: 实现头像预览加载逻辑

**Files:**
- Modify: `src/register.cpp`

- [ ] **Step 1: 添加必要的头文件包含**

```cpp
#include <QLabel>
#include <QPainter>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
```

- [ ] **Step 2: 在构造函数中查找并连接头像预览标签**

- [ ] **Step 3: 添加 onAvatarUrlChanged 槽函数实现**

- [ ] **Step 4: 在头文件中添加槽函数声明**

- [ ] **Step 5: 提交实现变更**

---

## Task 4: 构建并验证

**Files:**
- Build: 整个项目

- [ ] **Step 1: 清理并重新构建项目**

```bash
cmake --build build --target clean
cmake --build build --config Debug
```

Expected: 构建成功，无编译错误

- [ ] **Step 2: 运行应用程序验证 UI**

验证清单：
- [ ] 窗口尺寸为 420x600
- [ ] 三个分组标签（账户信息/联系方式/个人资料）正确显示
- [ ] 分组标签有背景色和装饰线
- [ ] 所有 8 个输入字段正常显示
- [ ] 验证码按钮内嵌在手机号右侧
- [ ] 头像预览显示默认头像（圆形）
- [ ] 昵称和头像URL并排显示
- [ ] 注册按钮样式正确
- [ ] 登录链接可点击

- [ ] **Step 3: 提交验证结果**

---

## Task 5: 代码审查与清理

- [ ] **Step 1: 删除备份文件**

- [ ] **Step 2: 检查代码风格一致性**

- [ ] **Step 3: 最终提交**

---

**计划完成并保存至 `docs/superpowers/plans/2026-05-31-register-ui-redesign.md`**

**执行选项：**

1. **Subagent-Driven (推荐)** - 为每个 Task 分派独立子代理，Task 间审查，快速迭代

2. **Inline Execution** - 在当前会话使用 executing-plans 批量执行

**选择哪种方式？**
