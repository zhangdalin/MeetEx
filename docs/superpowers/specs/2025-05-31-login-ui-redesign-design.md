# 登录界面重设计方案

**日期：** 2025-05-31  
**目标：** 参照注册界面重新设计登录界面，保持功能完整，风格统一

## 当前登录界面功能清单

- **账号登录标签页**
  - 账号输入框（用户名）
  - 密码输入框（密文显示）
- **手机登录标签页**
  - 手机号输入框
  - 验证码输入框 + 获取验证码按钮
- **底部操作区**
  - 登录按钮
  - 忘记密码链接
  - 注册链接
- **信号槽连接**
  - `loginBtn.clicked()` → `onLogin()`
  - `registerLinkLabel.linkActivated()` → `onRegisterLink()`
  - `sendCodeBtn.clicked()` → `onSendCode()`
  - `forgotPasswordLabel.linkActivated()` → `onForgotPassword()`

## 设计决策

**选定方案：A（保留标签页）**

理由：
- 功能完整保留，无需改动业务逻辑
- 用户操作习惯不变
- 仅需调整视觉样式即可达到统一

## UI 规格

### 窗口尺寸
- 宽度：420px（与注册页一致）
- 高度：600px（与注册页一致）

### 布局结构
```
┌─────────────────────────────────────┐
│         [顶部弹性间距]              │
├─────────────────────────────────────┤
│         登录 MeetEx                 │  ← 标题（18pt 加粗居中）
├─────────────────────────────────────┤
│         [间距 25px]                 │
├─────────────────────────────────────┤
│  ┌─────────────────────────────┐   │
│  │  账号登录  │  手机登录       │   │  ← QTabWidget
│  ├─────────────────────────────┤   │
│  │                             │   │
│  │  账号: [________________]   │   │  ← QFormLayout
│  │                             │   │     标签右对齐
│  │  密码: [________________]   │   │     垂直间距 12px
│  │                             │   │
│  └─────────────────────────────┘   │
├─────────────────────────────────────┤
│         [间距 30px]                 │
├─────────────────────────────────────┤
│        [  登 录  ]                  │  ← 蓝色按钮
│         (min-height: 40px)          │     (#0078d4)
├─────────────────────────────────────┤
│         [间距 15px]                 │
├─────────────────────────────────────┤
│      忘记密码？  |  去注册          │  ← 底部链接居中
├─────────────────────────────────────┤
│         [底部弹性间距]              │
└─────────────────────────────────────┘
```

### 样式规范

**标签页样式**
```css
QTabWidget::pane {
    border: 1px solid #e0e0e0;
    border-radius: 4px;
    background-color: white;
}
QTabBar::tab {
    padding: 8px 20px;
    margin-right: 2px;
    border: none;
    background-color: #f5f5f5;
}
QTabBar::tab:selected {
    background-color: #0078d4;
    color: white;
}
QTabBar::tab:hover:!selected {
    background-color: #e0e0e0;
}
```

**表单样式**
```css
/* 与注册页一致 */
QLabel {
    color: #333333;
}
QLineEdit {
    padding: 6px 10px;
    border: 1px solid #cccccc;
    border-radius: 4px;
    min-height: 20px;
}
QLineEdit:focus {
    border-color: #0078d4;
}
```

**登录按钮样式**
```css
/* 与注册页蓝色按钮完全一致 */
QPushButton {
    background-color: #0078d4;
    color: white;
    border: none;
    border-radius: 4px;
    font-size: 11pt;
    font-weight: bold;
    min-height: 40px;
}
QPushButton:hover {
    background-color: #005a9e;
}
QPushButton:pressed {
    background-color: #004578;
}
```

**获取验证码按钮样式**
```css
/* 与注册页一致 */
QPushButton {
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
}
```

**链接样式**
```css
QLabel {
    color: #0078d4;
}
QLabel:hover {
    color: #005a9e;
    text-decoration: underline;
}
```

### 布局参数

| 元素 | 参数 |
|------|------|
| 主布局边距 | left: 25, top: 20, right: 25, bottom: 20 |
| 标题与标签页间距 | 25px |
| 标签页与按钮间距 | 30px |
| 按钮与链接间距 | 15px |
| 表单水平间距 | 10px |
| 表单垂直间距 | 12px |
| 标签对齐 | Qt::AlignRight \| Qt::AlignVCenter |

## 实现要点

1. **保留所有现有信号槽连接**
   - `onLogin()`
   - `onRegisterLink()`
   - `onSendCode()`
   - `onForgotPassword()`

2. **保留所有控件 objectName**
   - 确保现有代码无需修改即可工作

3. **资源文件**
   - 移除登录按钮的锁图标（与注册页简洁风格一致）

## 验收标准

- [ ] 窗口尺寸与注册页一致（420x600）
- [ ] 所有现有功能正常工作
- [ ] 视觉风格与注册页统一
- [ ] 表单布局间距与注册页一致
- [ ] 按钮样式与注册页一致
- [ ] 底部链接居中对齐
