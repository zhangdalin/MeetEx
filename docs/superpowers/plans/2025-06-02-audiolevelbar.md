# AudioLevelBar 样式重设计实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为 JoinMeeting 界面的 audioLevelBar 实现圆角胶囊条样式和峰值保持功能

**Architecture:** 使用 Qt 样式表（QSS）自定义 QProgressBar 外观，添加 QLabel 作为峰值标记，在 updateAudioLevel() 中实现峰值检测和衰减逻辑

**Tech Stack:** Qt 6, C++17, CMake, QSS

---

## 文件结构

| 文件 | 操作 | 说明 |
|------|------|------|
| `ui/joinmeeting.ui` | 修改 | 更新 audioLevelBar 尺寸和样式，添加峰值标记 QLabel |
| `src/joinmeeting.h` | 修改 | 添加峰值检测相关成员变量 |
| `src/joinmeeting.cpp` | 修改 | 实现峰值检测和峰值标记更新逻辑 |

---

### Task 1: 修改 ui/joinmeeting.ui - 更新 audioLevelBar 尺寸

**Files:**
- Modify: `ui/joinmeeting.ui` (audioLevelBar 部分，约第 406-422 行)

- [ ] **Step 1: 修改 minimumSize 高度为 10px**

找到 `audioLevelBar` 的 `minimumSize` 属性，修改为：

```xml
<widget class="QProgressBar" name="audioLevelBar">
 <property name="minimumSize">
  <size>
   <width>100</width>
   <height>10</height>
  </size>
 </property>
 <!-- 保留其他属性 -->
</widget>
```

- [ ] **Step 2: 编译验证尺寸修改**

运行: `cmake --build build --config Debug 2>&1 | head -20`

预期: 编译成功，无 joinmeeting.ui 相关错误

- [ ] **Step 3: 提交尺寸修改**

```bash
git add ui/joinmeeting.ui
git commit -m "feat(ui): 设置 audioLevelBar 高度为 10px"
```

---

### Task 2: 修改 ui/joinmeeting.ui - 添加 QSS 样式表

**Files:**
- Modify: `ui/joinmeeting.ui` (audioLevelBar styleSheet 属性)

- [ ] **Step 1: 添加 QSS 样式表属性**

在 `audioLevelBar` 的 `maximum` 和 `value` 属性后添加 `styleSheet`：

```xml
<widget class="QProgressBar" name="audioLevelBar">
 <!-- ... 其他属性 ... -->
 <property name="maximum">
  <number>100</number>
 </property>
 <property name="value">
  <number>0</number>
 </property>
 <property name="styleSheet">
  <string notr="true">QProgressBar {
    background-color: #e9ecef;
    border-radius: 5px;
    border: none;
    text-align: center;
}
QProgressBar::chunk {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
        stop:0 #0078d4, stop:0.5 #2b88d8, stop:1 #0078d4);
    border-radius: 5px;
}</string>
 </property>
 <property name="textVisible">
  <bool>false</bool>
 </property>
</widget>
```

注意：`border-radius` 设为 5px（高度 10px 的一半）形成全圆角胶囊效果。

- [ ] **Step 2: 编译验证样式表**

运行: `cmake --build build --config Debug 2>&1 | head -20`

预期: 编译成功

- [ ] **Step 3: 运行应用验证视觉效果**

运行: `./build/Debug/MeetEx.exe`

验证:
- audioLevelBar 显示为圆角胶囊形状
- 背景为浅灰色 (#e9ecef)
- 进度条为蓝色渐变

- [ ] **Step 4: 提交样式表修改**

```bash
git add ui/joinmeeting.ui
git commit -m "feat(ui): 为 audioLevelBar 添加胶囊条 QSS 样式"
```

---

### Task 3: 修改 ui/joinmeeting.ui - 添加峰值标记 QLabel

**Files:**
- Modify: `ui/joinmeeting.ui` (在 audioLevelBar 后添加 peakLevelLabel)

- [ ] **Step 1: 在 audioLevelBar 后添加峰值标记 QLabel**

在 `audioLevelBar` widget 后、 `testSpeakerBtn` 前添加：

```xml
<item>
 <widget class="QLabel" name="peakLevelLabel">
  <property name="minimumSize">
   <size>
    <width>3</width>
    <height>10</height>
   </size>
  </property>
  <property name="maximumSize">
   <size>
    <width>3</width>
    <height>10</height>
   </size>
  </property>
  <property name="styleSheet">
   <string notr="true">background-color: #106ebe; border-radius: 2px;</string>
  </property>
  <property name="visible">
   <bool>false</bool>
  </property>
 </widget>
</item>
```

注意：峰值标记初始状态为隐藏 (`visible` 设为 `false`)。

- [ ] **Step 2: 调整布局间距**

修改 audioLevelBar 的 sizePolicy，使其可扩展：

```xml
<property name="sizePolicy">
 <sizepolicy hsizetype="Expanding" vsizetype="Fixed">
  <horstretch>0</horstretch>
  <verstretch>0</verstretch>
 </sizepolicy>
</property>
```

- [ ] **Step 3: 编译验证布局**

运行: `cmake --build build --config Debug 2>&1 | head -30`

预期: 编译成功，无布局相关警告

- [ ] **Step 4: 提交峰值标记添加**

```bash
git add ui/joinmeeting.ui
git commit -m "feat(ui): 添加峰值标记 QLabel"
```

---

### Task 4: 修改 src/joinmeeting.h - 添加峰值检测成员变量

**Files:**
- Modify: `src/joinmeeting.h`

- [ ] **Step 1: 在类定义中添加峰值检测成员变量**

在 `src/joinmeeting.h` 的 `private:` 区域，在现有成员变量后添加：

```cpp
private:
    Ui::JoinMeeting *ui;
    
    // 状态
    bool isVideoPreviewRunning_;
    QTimer *audioLevelTimer_;
    
    // 音频峰值检测 (新增)
    int currentAudioLevel_;      // 当前音频电平 (0-100)
    int peakLevel_;              // 峰值电平 (0-100)
    int peakHoldCounter_;        // 峰值保持计数器 (ms)
    static constexpr int PEAK_HOLD_DURATION = 300;  // 峰值保持时长 (ms)
    
    // ... 其他成员函数 ...
```

- [ ] **Step 2: 添加 updatePeakIndicator 函数声明**

在 `private:` 区域添加函数声明：

```cpp
    void updatePeakIndicator();  // 更新峰值标记位置和透明度
```

- [ ] **Step 3: 添加 resizeEvent 重写声明**

在 `protected:` 区域添加：

```cpp
protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;  // 新增
```

- [ ] **Step 4: 编译验证头文件**

运行: `cmake --build build --config Debug 2>&1 | grep -i "error" | head -10`

预期: 无错误

- [ ] **Step 5: 提交头文件修改**

```bash
git add src/joinmeeting.h
git commit -m "feat(joinmeeting): 添加音频峰值检测成员变量和函数声明"
```

---

### Task 5: 修改 src/joinmeeting.cpp - 初始化峰值检测变量

**Files:**
- Modify: `src/joinmeeting.cpp` (构造函数初始化列表)

- [ ] **Step 1: 修改构造函数初始化列表**

找到构造函数：

```cpp
JoinMeeting::JoinMeeting(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::JoinMeeting)
    , isVideoPreviewRunning_(false)
    , audioLevelTimer_(nullptr)
{
```

修改为：

```cpp
JoinMeeting::JoinMeeting(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::JoinMeeting)
    , isVideoPreviewRunning_(false)
    , audioLevelTimer_(nullptr)
    , currentAudioLevel_(0)
    , peakLevel_(0)
    , peakHoldCounter_(0)
{
```

- [ ] **Step 2: 添加必要的头文件包含**

在 `src/joinmeeting.cpp` 顶部添加：

```cpp
#include <algorithm>  // for std::min, std::max
```

- [ ] **Step 3: 编译验证初始化**

运行: `cmake --build build --config Debug 2>&1 | grep -i "joinmeeting" | head -10`

预期: 无错误或警告

- [ ] **Step 4: 提交初始化修改**

```bash
git add src/joinmeeting.cpp
git commit -m "feat(joinmeeting): 初始化音频峰值检测变量"
```

---

### Task 6: 修改 src/joinmeeting.cpp - 实现峰值检测逻辑

**Files:**
- Modify: `src/joinmeeting.cpp` (updateAudioLevel 函数，约第 309-314 行)

- [ ] **Step 1: 重写 updateAudioLevel 函数**

找到现有实现并替换为：

```cpp
void JoinMeeting::updateAudioLevel()
{
    // TODO: 从 MediaEngine 获取实时音频电平
    // 模拟音频电平 (0-100)
    currentAudioLevel_ = std::rand() % 60 + 10;  // 10-70 范围
    
    // 更新进度条
    ui->audioLevelBar->setValue(currentAudioLevel_);
    
    // 峰值检测逻辑
    if (currentAudioLevel_ > peakLevel_) {
        // 新峰值
        peakLevel_ = currentAudioLevel_;
        peakHoldCounter_ = PEAK_HOLD_DURATION;  // 重置保持计数器
    } else if (peakHoldCounter_ > 0) {
        // 保持峰值，递减计数器
        peakHoldCounter_ -= 100;  // 减去更新间隔 (100ms)
    } else {
        // 峰值跟随下降
        peakLevel_ = currentAudioLevel_;
    }
    
    // 更新峰值标记显示
    updatePeakIndicator();
}
```

- [ ] **Step 2: 实现 updatePeakIndicator 函数**

在 `updateAudioLevel` 函数后添加：

```cpp
void JoinMeeting::updatePeakIndicator()
{
    // 获取进度条几何信息
    int barWidth = ui->audioLevelBar->width();
    int barHeight = ui->audioLevelBar->height();
    
    // 计算峰值标记位置（相对于进度条）
    int peakPos = (peakLevel_ * barWidth) / 100;
    
    // 限制位置在进度条范围内
    peakPos = std::min(peakPos, barWidth - 3);  // 减去标记宽度
    peakPos = std::max(peakPos, 0);
    
    // 计算透明度
    float opacity = 0.0f;
    if (peakHoldCounter_ > 0) {
        // 峰值保持期间，透明度从 0.8 线性衰减
        opacity = 0.8f * static_cast<float>(peakHoldCounter_) / PEAK_HOLD_DURATION;
    } else {
        // 峰值跟随模式，低透明度
        opacity = 0.3f;
    }
    
    // 更新峰值标记样式和位置
    if (opacity > 0.05f) {
        // 设置位置（相对于父布局，需要计算绝对位置）
        QPoint barPos = ui->audioLevelBar->mapToParent(QPoint(0, 0));
        ui->peakLevelLabel->move(barPos.x() + peakPos, barPos.y());
        
        // 设置透明度（通过 rgba）
        int alpha = static_cast<int>(opacity * 255);
        QString style = QString("background-color: rgba(16, 110, 190, %1); border-radius: 2px;")
                        .arg(alpha);
        ui->peakLevelLabel->setStyleSheet(style);
        ui->peakLevelLabel->setFixedSize(3, barHeight);
        ui->peakLevelLabel->setVisible(true);
    } else {
        ui->peakLevelLabel->setVisible(false);
    }
}
```

- [ ] **Step 3: 编译验证逻辑实现**

运行: `cmake --build build --config Debug 2>&1 | head -30`

预期: 编译成功，无错误

- [ ] **Step 4: 提交峰值检测实现**

```bash
git add src/joinmeeting.cpp
git commit -m "feat(joinmeeting): 实现音频峰值检测和峰值标记更新逻辑"
```

---

### Task 7: 实现 resizeEvent 处理窗口大小变化

**Files:**
- Modify: `src/joinmeeting.cpp`

- [ ] **Step 1: 实现 resizeEvent**

在 `src/joinmeeting.cpp` 中添加：

```cpp
void JoinMeeting::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    
    // 窗口大小变化时更新峰值标记位置
    if (ui->peakLevelLabel && ui->peakLevelLabel->isVisible()) {
        updatePeakIndicator();
    }
}
```

- [ ] **Step 2: 编译验证**

运行: `cmake --build build --config Debug`

预期: 编译成功

- [ ] **Step 3: 提交 resize 处理**

```bash
git add src/joinmeeting.cpp
git commit -m "feat(joinmeeting): 处理窗口大小变化时更新峰值标记位置"
```

---

### Task 8: 最终验证和测试

**Files:**
- Test: 运行应用验证所有功能

- [ ] **Step 1: 完整编译**

运行: `cmake --build build --config Debug`

预期: 编译成功，无错误无警告

- [ ] **Step 2: 运行应用验证视觉效果**

运行: `./build/Debug/MeetEx.exe`

验证清单：
- [ ] audioLevelBar 显示为 10px 高的圆角胶囊条
- [ ] 背景色为浅灰色 (#e9ecef)
- [ ] 进度条有蓝色渐变效果
- [ ] 峰值标记（蓝色细线）在音量峰值时出现
- [ ] 峰值标记在 300ms 后淡出
- [ ] 调整窗口大小后峰值标记位置正确

- [ ] **Step 3: 最终提交**

```bash
git log --oneline -5  # 查看提交历史
git status            # 确认无未提交修改
```

预期: 所有修改已提交，工作区干净

---

## 自审查结果

### 1. 规格覆盖检查

| 设计文档需求 | 实现任务 |
|-------------|---------|
| 高度 10px | Task 1 ✓ |
| 圆角胶囊形状 | Task 2 ✓ |
| 蓝色渐变 | Task 2 ✓ |
| 峰值标记 QLabel | Task 3 ✓ |
| 峰值检测逻辑 | Task 6 ✓ |
| 300ms 峰值保持 | Task 6 ✓ |
| 窗口大小变化处理 | Task 7 ✓ |

### 2. 占位符扫描

- 无 "TBD", "TODO"（除 MediaEngine 集成注释外）
- 所有代码片段完整
- 命令和预期输出明确

### 3. 类型一致性检查

- `peakLevel_` 始终为 int (0-100)
- `peakHoldCounter_` 始终为 int (ms)
- `opacity` 计算使用 float，最终转为 int (0-255)
- 所有函数签名在 .h 和 .cpp 中一致

---

## 执行选项

**计划已完成并保存到 `docs/superpowers/plans/2025-06-02-audiolevelbar.md`。两个执行选项：**

**1. Subagent-Driven（推荐）** - 每个任务派生新的子代理，任务间审查，快速迭代

**2. Inline Execution** - 在此会话中执行任务，批量执行并设置检查点

**请选择执行方式。**
