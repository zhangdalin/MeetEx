# AudioLevelBar 样式重设计规格文档

> **日期:** 2025-06-02  
> **主题:** JoinMeeting 界面音频电平指示器样式重设计  
> **状态:** 已确认，待实现

## 概述

重新设计 MeetEx 应用程序 JoinMeeting 界面中的 `audioLevelBar` 组件，采用现代简约风格，添加峰值保持功能，提升用户体验和视觉美感。

## 当前状态

- **组件类型:** `QProgressBar`
- **位置:** `ui/joinmeeting.ui` 中音频设备选择区域
- **更新频率:** 每 100ms 通过 `updateAudioLevel()` 更新
- **当前问题:** 使用默认样式，无自定义视觉效果，缺乏峰值指示

## 设计方案

### 视觉风格: 圆角胶囊条

采用现代简约风格的胶囊形进度条设计。

#### 尺寸规格

| 属性 | 值 | 说明 |
|------|-----|------|
| 高度 | 10px | 适中的视觉存在感 |
| 圆角 | 10px | 全圆角胶囊形状 |
| 最小宽度 | 100px | 保证足够的显示精度 |
| 与设备标签间距 | 12px | 保持视觉呼吸感 |

#### 颜色方案

| 元素 | 颜色值 | 说明 |
|------|--------|------|
| 背景色 | `#e9ecef` | 浅灰色背景槽 |
| 进度条渐变起点 | `#0078d4` | 主蓝色 |
| 进度条渐变中点 | `#2b88d8` | 浅蓝色（光泽效果） |
| 进度条渐变终点 | `#0078d4` | 回到主蓝色 |
| 峰值标记 | `#106ebe` | 深蓝色标记 |
| 禁用状态 | `#999999` | 灰色 |
| 静音状态 | `#cccccc` | 浅灰色 |

#### 渐变公式

```css
background: linear-gradient(90deg, #0078d4 0%, #2b88d8 50%, #0078d4 100%);
```

### 峰值保持功能

在进度条上显示一个小标记，指示最近的最大音量电平。

#### 峰值标记规格

| 属性 | 值 | 说明 |
|------|-----|------|
| 宽度 | 3px | 细线标记 |
| 高度 | 100% | 贯穿整个进度条 |
| 圆角 | 2px | 微圆角 |
| 初始透明度 | 0.8 | 明显可见 |
| 显示时长 | 300ms | 保持时间 |
| 衰减方式 | 淡出 | opacity 降至 0 |

#### 峰值检测逻辑

```cpp
// 伪代码
if (currentLevel > peakLevel) {
    peakLevel = currentLevel;
    peakOpacity = 0.8;
    peakHoldTimer = 300ms;
} else if (peakHoldTimer > 0) {
    peakHoldTimer -= updateInterval;
    peakOpacity = 0.8 * (peakHoldTimer / 300ms);
} else {
    peakLevel = currentLevel;  // 跟随当前值下降
}
```

### 动画效果

| 动画 | 参数 | 说明 |
|------|------|------|
| 电平变化 | 100ms ease-out | 平滑过渡，避免跳变 |
| 峰值衰减 | 300ms 线性淡出 | 自然消失效果 |
| 状态切换 | 即时 | 禁用/静音状态立即响应 |

## 实现方案

### 推荐方案: QSS 样式表

使用 Qt 样式表（QSS）实现，无需自定义控件类。

#### 优点

- 无需额外 C++ 代码
- 与 Qt Designer 完全兼容
- 易于维护和修改
- 性能开销最小

#### 缺点

- 峰值标记需要额外的 QLabel 组件
- 复杂动画受限

### 替代方案: 自定义控件

创建 `AudioLevelBar` 自定义控件类继承 `QWidget`，重写 `paintEvent`。

#### 优点

- 完全自定义绘制
- 可以实现更复杂的动画
- 峰值标记集成在控件内部

#### 缺点

- 增加代码复杂度
- 需要额外测试覆盖
- 与 Qt Designer 集成较麻烦

**决策:** 采用 QSS 方案，满足当前需求且实现成本最低。

## 文件变更清单

### 1. ui/joinmeeting.ui

**变更内容:**
- 修改 `audioLevelBar` 的最小高度为 10px
- 添加 QSS 样式表属性
- 添加峰值标记 QLabel (`peakLevelLabel`)

**关键属性:**
```xml
<property name="minimumSize">
 <size>
  <width>100</width>
  <height>10</height>
 </size>
</property>
<property name="styleSheet">
 <string notr="true">QProgressBar {
    background-color: #e9ecef;
    border-radius: 10px;
    border: none;
 }
 QProgressBar::chunk {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
        stop:0 #0078d4, stop:0.5 #2b88d8, stop:1 #0078d4);
    border-radius: 10px;
 }</string>
</property>
```

### 2. src/joinmeeting.h

**新增成员变量:**
```cpp
// 音频峰值检测
int peakLevel_;              // 当前峰值电平 (0-100)
int peakHoldCounter_;        // 峰值保持计数器 (ms)
QTimer *peakDecayTimer_;     // 峰值衰减定时器 (可选)
```

### 3. src/joinmeeting.cpp

**修改 updateAudioLevel():**
```cpp
void JoinMeeting::updateAudioLevel()
{
    // TODO: 从 MediaEngine 获取实时音频电平
    int currentLevel = std::rand() % 50;  // 模拟数据
    
    // 更新进度条
    ui->audioLevelBar->setValue(currentLevel);
    
    // 峰值检测
    if (currentLevel > peakLevel_) {
        peakLevel_ = currentLevel;
        peakHoldCounter_ = 300;  // 重置保持计数器
        updatePeakIndicator();
    } else if (peakHoldCounter_ > 0) {
        peakHoldCounter_ -= 100;  // 100ms 更新间隔
        updatePeakIndicator();
    } else {
        // 峰值跟随下降
        peakLevel_ = currentLevel;
        updatePeakIndicator();
    }
}

void JoinMeeting::updatePeakIndicator()
{
    // 计算峰值标记位置
    int barWidth = ui->audioLevelBar->width();
    int peakPos = (peakLevel_ * barWidth) / 100;
    
    // 更新 QLabel 位置
    ui->peakLevelLabel->move(
        ui->audioLevelBar->x() + peakPos,
        ui->audioLevelBar->y()
    );
    
    // 计算透明度
    float opacity = (peakHoldCounter_ > 0) 
        ? (0.8f * peakHoldCounter_ / 300.0f) 
        : 0.0f;
    
    // 应用透明度
    QString style = QString("background-color: rgba(16, 110, 190, %1); 
                           border-radius: 2px;")
                    .arg(opacity);
    ui->peakLevelLabel->setStyleSheet(style);
    ui->peakLevelLabel->setVisible(opacity > 0.05f);
}
```

## 状态处理

### 禁用状态

当麦克风设备被禁用时:
- 进度条值设为 0
- 进度条颜色变为 `#999999`
- 峰值标记隐藏

### 静音状态

当用户静音麦克风时:
- 进度条继续更新（显示输入电平）
- 进度条颜色变为 `#cccccc`（浅灰）
- 或在 UI 上显示静音图标

## 测试要点

1. **视觉验证:**
   - 胶囊形状正确显示
   - 渐变效果平滑
   - 峰值标记位置准确

2. **功能验证:**
   - 电平更新频率正确 (100ms)
   - 峰值检测响应及时
   - 峰值衰减 300ms 后消失

3. **边界情况:**
   - 电平为 0 时的显示
   - 电平为 100 时的显示
   - 快速电平变化时的平滑度

## 参考

- Qt QProgressBar 文档: https://doc.qt.io/qt-6/qprogressbar.html
- Qt 样式表参考: https://doc.qt.io/qt-6/stylesheet-reference.html
- JoinMeeting 实现计划: `docs/superpowers/plans/2025-06-01-joinmeeting.md`
