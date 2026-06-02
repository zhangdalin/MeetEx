# 登录界面重设计实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 参照注册界面重设计登录界面，保持功能完整，风格统一

**Architecture：** 修改 Qt Designer UI 文件 (login.ui)，调整布局、间距、样式以匹配注册页设计，保留所有现有信号槽连接

**Tech Stack：** Qt 6, Qt Designer UI XML

---

## 文件结构

| 文件 | 操作 | 说明 |
|------|------|------|
| `ui/login.ui` | 修改 | 主 UI 文件，调整布局、间距、样式 |
| `src/login.h` | 读取确认 | 确认槽函数声明（无修改） |
| `src/login.cpp` | 读取确认 | 确认槽函数实现（无修改） |

---

### Task 1: 读取并分析现有代码

**目的：** 确认现有信号槽连接和槽函数，确保 UI 修改不会破坏功能

**Files:**
- Read: `src/login.h`
- Read: `src/login.cpp`

- [ ] **Step 1: 读取 login.h 确认槽函数声明**

检查 `login.h` 中是否声明了以下槽函数：
- `onLogin()`
- `onRegisterLink()`
- `onSendCode()`
- `onForgotPassword()`

- [ ] **Step 2: 读取 login.cpp 确认槽函数实现**

验证槽函数实现存在且正常工作。

- [ ] **Step 3: 确认控件 objectName 列表**

确保以下控件名称与现有代码匹配：
- `accountLineEdit`
- `passwordLineEdit`
- `phoneLineEdit`
- `codeLineEdit`
- `sendCodeBtn`
- `loginBtn`
- `registerLinkLabel`
- `forgotPasswordLabel`
- `loginTabWidget`

---

### Task 2: 更新窗口尺寸和布局边距

**Files:**
- Modify: `ui/login.ui`

- [ ] **Step 1: 更新窗口尺寸为 420x600**

将第 9-10 行从：
```xml
<width>400</width>
<height>550</height>
```
修改为：
```xml
<width>420</width>
<height>600</height>
```

- [ ] **Step 2: 更新主布局边距**

在第 16 行的 `<layout class="QVBoxLayout" name="verticalLayout">` 后添加边距属性：

```xml
<property name="spacing">
 <number>0</number>
</property>
<property name="leftMargin">
 <number>25</number>
</property>
<property name="topMargin">
 <number>20</number>
</property>
<property name="rightMargin">
 <number>25</number>
</property>
<property name="bottomMargin">
 <number>20</number>
</property>
```

---

### Task 3: 更新标题与标签页之间的间距

**Files:**
- Modify: `ui/login.ui`

- [ ] **Step 1: 将标题下方的弹性 spacer 改为固定高度 spacer**

找到第 41-46 行的 `verticalSpacer_title`，替换为固定高度 spacer：

```xml
<item>
 <spacer name="verticalSpacer_afterTitle">
  <property name="orientation">
   <enum>Qt::Vertical</enum>
  </property>
  <property name="sizeHint" stdset="0">
   <size>
    <width>20</width>
    <height>25</height>
   </size>
  </property>
 </spacer>
</item>
```

---

### Task 4: 更新表单布局样式

**Files:**
- Modify: `ui/login.ui`

- [ ] **Step 1: 更新账号登录表单的 FormLayout 属性**

找到第 58 行的 `<layout class="QFormLayout" name="accountFormLayout">`，添加属性：

```xml
<property name="labelAlignment">
 <set>Qt::AlignRight|Qt::AlignVCenter</set>
</property>
<property name="formAlignment">
 <set>Qt::AlignLeading|Qt::AlignTop</set>
</property>
<property name="horizontalSpacing">
 <number>10</number>
</property>
<property name="verticalSpacing">
 <number>12</number>
</property>
```

- [ ] **Step 2: 更新手机登录表单的 FormLayout 属性**

找到第 100 行的 `<layout class="QFormLayout" name="phoneFormLayout">`，添加相同属性：

```xml
<property name="labelAlignment">
 <set>Qt::AlignRight|Qt::AlignVCenter</set>
</property>
<property name="formAlignment">
 <set>Qt::AlignLeading|Qt::AlignTop</set>
</property>
<property name="horizontalSpacing">
 <number>10</number>
</property>
<property name="verticalSpacing">
 <number>12</number>
</property>
```

- [ ] **Step 3: 更新获取验证码按钮样式**

找到第 132-136 行的 `sendCodeBtn`，添加 styleSheet 属性：

```xml
<property name="styleSheet">
 <string notr="true">QPushButton {
    border: none;
    background: transparent;
    color: #0078d4;
    font-size: 9pt;
    padding: 0 5px;
}
QPushButton:hover {
    color: #005a9e;
}
QPushButton:disabled {
    color: #999999;
}</string>
</property>
```

---

### Task 5: 更新按钮和链接区域

**Files:**
- Modify: `ui/login.ui`

- [ ] **Step 1: 更新标签页与按钮之间的 spacer**

找到第 147-151 行的 `verticalSpacer_form`，替换为固定高度 spacer：

```xml
<item>
 <spacer name="verticalSpacer_beforeButton">
  <property name="orientation">
   <enum>Qt::Vertical</enum>
  </property>
  <property name="sizeHint" stdset="0">
   <size>
    <width>20</width>
    <height>30</height>
   </size>
  </property>
 </spacer>
</item>
```

- [ ] **Step 2: 更新登录按钮样式**

找到第 154-168 行的 `loginBtn`，替换为：

```xml
<widget class="QPushButton" name="loginBtn">
 <property name="minimumSize">
  <size>
   <width>0</width>
   <height>40</height>
  </size>
 </property>
 <property name="styleSheet">
  <string notr="true">QPushButton {
    background-color: #0078d4;
    color: white;
    border: none;
    border-radius: 4px;
    font-size: 11pt;
    font-weight: bold;
}
QPushButton:hover {
    background-color: #005a9e;
}
QPushButton:pressed {
    background-color: #004578;
}</string>
 </property>
 <property name="text">
  <string>登 录</string>
 </property>
</widget>
```

（注意：移除了 icon 属性以匹配注册页简洁风格）

- [ ] **Step 3: 在按钮后添加固定间距**

在第 169 行后添加 spacer：

```xml
<item>
 <spacer name="verticalSpacer_afterButton">
  <property name="orientation">
   <enum>Qt::Vertical</enum>
  </property>
  <property name="sizeHint" stdset="0">
   <size>
    <width>20</width>
    <height>15</height>
   </size>
  </property>
 </spacer>
</item>
```

---

### Task 6: 更新底部链接布局

**Files:**
- Modify: `ui/login.ui`

- [ ] **Step 1: 将链接布局改为居中对齐**

找到第 171-199 行的 `linkLayout`，将整个水平布局替换为垂直居中布局：

```xml
<item>
 <layout class="QHBoxLayout" name="linkLayout">
  <property name="spacing">
   <number>20</number>
  </property>
  <item>
   <spacer name="horizontalSpacer_left">
    <property name="orientation">
     <enum>Qt::Horizontal</enum>
    </property>
   </spacer>
  </item>
  <item>
   <widget class="QLabel" name="forgotPasswordLabel">
    <property name="text">
     <string>&lt;a href=&quot;#forgot&quot;&gt;忘记密码？&lt;/a&gt;</string>
    </property>
    <property name="alignment">
     <set>Qt::AlignCenter</set>
    </property>
   </widget>
  </item>
  <item>
   <widget class="QLabel" name="separatorLabel">
    <property name="text">
     <string>|</string>
    </property>
    <property name="styleSheet">
     <string notr="true">color: #cccccc;</string>
    </property>
   </widget>
  </item>
  <item>
   <widget class="QLabel" name="registerLinkLabel">
    <property name="text">
     <string>&lt;a href=&quot;#register&quot;&gt;去注册&lt;/a&gt;</string>
    </property>
    <property name="alignment">
     <set>Qt::AlignCenter</set>
    </property>
   </widget>
  </item>
  <item>
   <spacer name="horizontalSpacer_right">
    <property name="orientation">
     <enum>Qt::Horizontal</enum>
    </property>
   </spacer>
  </item>
 </layout>
</item>
```

---

### Task 7: 构建和视觉验证

**Files:**
- Build: 整个项目

- [ ] **Step 1: 清理并重新构建项目**

```bash
cmake --build build --target clean
cmake -B build -S . -DCMAKE_PREFIX_PATH="C:/Qt/6.10.2/msvc2022_64"
cmake --build build --config Debug
```

- [ ] **Step 2: 运行应用程序验证登录界面**

```bash
./build/Debug/MeetEx.exe
```

验证清单：
- [ ] 窗口尺寸为 420x600
- [ ] 标签页样式正确（账号登录/手机登录）
- [ ] 表单标签右对齐
- [ ] 登录按钮为蓝色（#0078d4），高度 40px
- [ ] 获取验证码按钮为文字链接样式
- [ ] 底部链接居中显示
- [ ] 账号登录功能正常
- [ ] 手机登录功能正常
- [ ] 点击登录按钮触发 onLogin()
- [ ] 点击注册链接触发 onRegisterLink()
- [ ] 点击获取验证码触发 onSendCode()
- [ ] 点击忘记密码触发 onForgotPassword()

---

## 自审检查

**1. 规格覆盖检查：**
- ✅ 窗口尺寸 420x600 → Task 2
- ✅ 布局边距 25/20/25/20 → Task 2
- ✅ 表单标签右对齐 → Task 4
- ✅ 登录按钮蓝色样式 → Task 5
- ✅ 获取验证码按钮样式 → Task 4
- ✅ 底部链接居中 → Task 6
- ✅ 所有信号槽保留 → 确认控件 objectName 不变

**2. 占位符扫描：**
- ✅ 无 TBD/TODO
- ✅ 所有 XML 代码完整
- ✅ 所有命令明确

**3. 类型一致性：**
- ✅ 所有 objectName 与现有代码一致
- ✅ 信号槽连接名称保持不变

---

## 变更摘要

| 修改项 | 变更内容 |
|--------|----------|
| 窗口尺寸 | 400x550 → 420x600 |
| 主布局边距 | 添加 25/20/25/20 边距 |
| 标题间距 | 弹性 spacer → 固定 25px |
| 表单布局 | 添加标签右对齐、间距 12px |
| 获取验证码按钮 | 添加文字链接样式 |
| 登录按钮 | 添加蓝色样式，移除图标 |
| 按钮前间距 | 弹性 spacer → 固定 30px |
| 按钮后间距 | 添加固定 15px spacer |
| 底部链接 | 左右对齐 → 居中对齐 |
