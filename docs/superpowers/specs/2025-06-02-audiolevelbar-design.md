# AudioLevelBar 重设计文档

**日期**: 2025-06-02  
**主题**: JoinMeeting 界面音量指示器重构  
**状态**: 待实现

---

## 1. 设计目标

重新设计 `audioLevelBar` 控件，实现：
- 实时显示麦克风音量（范围 1-100）
- 采用网络信号强度风格的直角三角形指示器
- 右下角固定，斜边随音量动态延伸
- 颜色从绿色渐变到蓝色（音量越大越蓝）
- **半透明**三角形填充，不遮挡中央图标
- 中央放置 16x16 麦克风图标，始终完全可见

---

## 2. 视觉设计

### 2.1 布局结构

```
┌─────────────────────────────┐
│                             │
│        🔊 (图标)             │  ← 麦克风图标 16x16，始终在最上层
│                             │
│    ▲                        │  ← 半透明三角形
│   /│                        │     右下角固定
│  / │                        │     斜边动态延伸
│ /__│                        │
│                             │
└─────────────────────────────┘
```

### 2.2 三角形几何

- **直角位置**: 右下角 (widget width, widget height)
- **固定顶点**: 右下角
- **动态顶点**: 左下角随音量向中心移动
- **斜边顶点**: 上边缘随音量向右移动

**音量计算**:
```
当 volume = 0:   三角形不可见
当 volume = 100: 三角形充满整个控件区域
```

### 2.3 颜色渐变

| 音量值 | 颜色 | RGB |
|--------|------|-----|
| 1 | 绿色 | #4CAF50 (76, 175, 80) |
| 50 | 青绿 | #2196F3 (33, 150, 243) |
| 100 | 蓝色 | #1976D2 (25, 118, 210) |

使用 `QLinearGradient` 实现平滑渐变。

### 2.4 透明度

- 三角形填充: **60% 透明度** (alpha = 153)
- 图标: 100% 不透明
- 背景: 透明或浅色底

---

## 3. 技术方案

### 3.1 架构

创建自定义控件 `AudioLevelIndicator`:

```cpp
class AudioLevelIndicator : public QWidget {
    Q_OBJECT
public:
    explicit AudioLevelIndicator(QWidget *parent = nullptr);
    void setVolume(int volume);  // 0-100
    int volume() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    int volume_ = 0;
    QPixmap micIcon_;
    void drawTriangle(QPainter &painter);
    void drawIcon(QPainter &painter);
    QColor calculateColor(int volume);
};
```

### 3.2 绘制流程

1. **paintEvent** 中调用:
   - `drawTriangle()`: 绘制半透明渐变三角形
   - `drawIcon()`: 在中央绘制 16x16 麦克风图标

2. **三角形顶点计算**:
   ```cpp
   // 控件尺寸
   qreal w = width();
   qreal h = height();
   
   // 音量比例 (0.0 - 1.0)
   qreal ratio = volume_ / 100.0;
   
   // 三个顶点
   QPointF bottomRight(w, h);           // 右下角（直角）
   QPointF bottomLeft(w * (1 - ratio), h);  // 左下角（动态）
   QPointF topRight(w, h * (1 - ratio));    // 右上角（动态）
   ```

3. **渐变设置**:
   ```cpp
   QLinearGradient gradient(bottomLeft, topRight);
   gradient.setColorAt(0.0, QColor(76, 175, 80, 153));   // 绿色半透明
   gradient.setColorAt(0.5, QColor(33, 150, 243, 153));  // 蓝色半透明
   gradient.setColorAt(1.0, QColor(25, 118, 210, 153));  // 深蓝半透明
   ```

### 3.3 集成到 JoinMeeting

1. 替换 `ui/joinmeeting.ui` 中的 `QProgressBar` 为 `AudioLevelIndicator`
2. 更新 `src/joinmeeting.cpp` 中的音量更新逻辑:
   ```cpp
   void JoinMeeting::updateAudioLevel() {
       int level = MediaEngine::instance()->getAudioLevel();
       ui->audioLevelIndicator->setVolume(level);
   }
   ```

---

## 4. 尺寸规格

| 属性 | 值 |
|------|-----|
| 控件最小宽度 | 80px |
| 控件高度 | 40px |
| 图标尺寸 | 16x16px |
| 图标位置 | 中央 |
| 三角形透明度 | 60% |

---

## 5. 文件变更

### 新增文件
- `src/widgets/audio_level_indicator.h`
- `src/widgets/audio_level_indicator.cpp`

### 修改文件
- `src/joinmeeting.cpp`: 更新音量更新逻辑
- `src/joinmeeting.h`: 添加控件引用
- `ui/joinmeeting.ui`: 替换控件类型
- `CMakeLists.txt`: 添加新源文件

---

## 6. 测试要点

1. 音量从 0 到 100 变化时三角形平滑延伸
2. 颜色渐变从绿到蓝正确过渡
3. 图标始终位于中央且完全可见
4. **半透明效果正确**，不遮挡下层内容
5. 不同窗口大小下布局正确

---

## 7. 备选方案

如需简化实现，可考虑：
- 使用 `QProgressBar` + 自定义样式表（三角形效果受限）
- 预渲染三角形图片序列（增加资源文件）

---

**批准状态**: 待用户审核
