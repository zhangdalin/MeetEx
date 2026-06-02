# 注册页面 UI 重新设计规范

**日期**: 2026-05-31  
**主题**: 注册页面 UI 重新设计  
**状态**: 待实施

---

## 1. 设计目标

重新设计注册页面 UI，在保持所有现有功能的前提下，提升视觉层次、用户体验和表单可用性。

## 2. 功能保留清单

以下现有功能必须在重新设计后完整保留：

| 功能 | 控件 | 信号/槽 |
|------|------|---------|
| 账号输入 | `accountLineEdit` | - |
| 密码输入 | `passwordLineEdit` | 点击时短暂显示密码 |
| 确认密码输入 | `confirmPasswordLineEdit` | 点击时短暂显示密码 |
| 昵称输入 | `displayNameLineEdit` | - |
| 邮箱输入 | `emailLineEdit` | - |
| 手机号输入 | `phoneLineEdit` | - |
| 获取验证码按钮 | `sendCodeBtn` | `onSendCode()` |
| 验证码输入 | `codeLineEdit` | - |
| 头像URL输入 | `avatarLineEdit` | - |
| 注册按钮 | `registerBtn` | `onRegister()` |
| 登录链接 | `loginLinkLabel` | `onLoginLink()` |

## 3. 视觉设计规范

### 3.1 窗口尺寸

- **宽度**: 420px
- **高度**: 600px
- 最小高度确保所有字段无需滚动即可显示

### 3.2 配色方案

沿用项目现有配色，与登录页保持一致：

| 用途 | 颜色值 |
|------|--------|
| 主色调（按钮/链接） | `#0078d4` |
| 标题文字 | `#333333` |
| 标签文字 | `#666666` |
| 输入框边框 | `#cccccc` |
| 输入框焦点边框 | `#0078d4` |
| 分组标签背景 | `#f5f5f5` |
| 分组标签文字 | `#666666` |
| 默认头像背景 | `#e0e0e0` |

### 3.3 字体规范

| 元素 | 字号 | 字重 |
|------|------|------|
| 页面标题 | 18pt | Bold |
| 分组标签 | 10pt | Bold |
| 字段标签 | 10pt | Normal |
| 输入框文字 | 10pt | Normal |
| 按钮文字 | 11pt | Bold |
| 链接文字 | 10pt | Normal |

### 3.4 间距规范

- 页面顶部边距: 20px
- 标题与第一个分组间距: 25px
- 分组之间间距: 20px
- 分组内字段间距: 12px
- 分组内边距: 15px
- 最后一个分组与注册按钮间距: 30px
- 注册按钮与登录链接间距: 15px
- 页面底部边距: 20px

## 4. 布局设计

### 4.1 整体结构

```
┌─────────────────────────────┐  ◄── 420px
│                             │
│      [页面标题]             │     注册 MeetEx (18pt Bold, 居中)
│                             │
│    ━━━ 账户信息 ━━━         │     分组标签 (带背景色 #f5f5f5)
│                             │
│    账号: [__________]       │     账号输入框
│    密码: [__________]       │     密码输入框 (Password模式)
│    确认密码: [______]       │     确认密码输入框 (Password模式)
│                             │
│    ━━━ 联系方式 ━━━         │
│                             │
│    邮箱: [__________]       │     邮箱输入框
│    手机号: [____] [获取验证码]│     手机号 + 内嵌发送按钮
│    验证码: [________]       │     验证码输入框
│                             │
│    ━━━ 个人资料 ━━━         │
│                             │
│    ┌───┐                    │
│    │ 头│ 昵称: [________]   │     头像预览 + 昵称并排
│    │ 像│                    │
│    └───┘ URL: [__________]  │     头像URL输入
│                             │
│                             │
│        [    注 册    ]      │     注册按钮 (高度 40px)
│                             │
│      [已有账号？去登录]      │     登录链接
│                             │
└─────────────────────────────┘  ◄── 600px
```

### 4.2 分组设计

每个分组包含：
- **分组标签**: 居中显示，背景色 `#f5f5f5`，文字色 `#666666`，字号 10pt Bold
- **标签装饰**: 分组标签两侧使用短横线装饰（如 `━━━ 账户信息 ━━━`）

### 4.3 字段布局

所有字段使用 `QFormLayout`：
- 标签列宽度: 80px
- 输入框列: 剩余空间
- 字段垂直间距: 12px

### 4.4 特殊控件设计

#### 4.4.1 验证码区域

手机号和获取验证码按钮内嵌布局：

```
┌─────────────────────────────────────┐
│ 手机号: [_______________] [获取验证码] │
│          ↑ 手机号输入框   ↑ 内嵌按钮   │
└─────────────────────────────────────┘
```

- 发送按钮位于输入框内部右侧
- 按钮文字: "获取验证码" → 点击后显示倒计时 "60s"
- 按钮样式: 无边框，文字色 `#0078d4`

#### 4.4.2 头像预览区域

头像预览与昵称并排布局：

```
┌─────────────────────────────────────┐
│ ┌────┐                              │
│ │ 64 │  昵称: [__________________]   │
│ │ px │                              │
│ └────┘  URL:  [__________________]   │
│  圆形头像                             │
└─────────────────────────────────────┘
```

- 头像尺寸: 64x64px
- 头像形状: 圆形（通过样式表 `border-radius: 32px` 实现）
- 默认头像: 使用现有资源 `:/assets/user.png`
- 点击头像可打开 URL 输入框进行编辑

#### 4.4.3 密码输入框

保持现有设计：
- 默认模式: `QLineEdit::Password`
- 交互: 按住/点击时短暂切换为 `QLineEdit::Normal` 显示密码
- 无需额外可见性切换按钮

## 5. 样式表规范

### 5.1 分组标签样式

```css
QLabel#groupLabel {
    background-color: #f5f5f5;
    color: #666666;
    font-size: 10pt;
    font-weight: bold;
    padding: 5px 15px;
    border-radius: 3px;
}
```

### 5.2 输入框样式

```css
QLineEdit {
    border: 1px solid #cccccc;
    border-radius: 4px;
    padding: 8px;
    font-size: 10pt;
}

QLineEdit:focus {
    border-color: #0078d4;
    outline: none;
}

QLineEdit::placeholder {
    color: #999999;
}
```

### 5.3 获取验证码按钮样式

```css
QPushButton#sendCodeBtn {
    border: none;
    background: transparent;
    color: #0078d4;
    font-size: 9pt;
    padding: 0 5px;
}

QPushButton#sendCodeBtn:hover {
    color: #005a9e;
}

QPushButton#sendCodeBtn:disabled {
    color: #999999;
}
```

### 5.4 注册按钮样式

```css
QPushButton#registerBtn {
    background-color: #0078d4;
    color: white;
    border: none;
    border-radius: 4px;
    font-size: 11pt;
    font-weight: bold;
    min-height: 40px;
}

QPushButton#registerBtn:hover {
    background-color: #005a9e;
}

QPushButton#registerBtn:pressed {
    background-color: #004578;
}
```

### 5.5 头像预览样式

```css
QLabel#avatarPreview {
    min-width: 64px;
    min-height: 64px;
    max-width: 64px;
    max-height: 64px;
    border-radius: 32px;
    border: 2px solid #e0e0e0;
}
```

## 6. 交互行为

### 6.1 表单验证

保留现有验证逻辑：
- `validateAccount()` - 账号格式验证
- `validatePassword()` - 密码强度验证
- `validateEmail()` - 邮箱格式验证
- `validatePhone()` - 手机号格式验证
- `passwordsMatch()` - 密码一致性验证

### 6.2 验证码倒计时

保留现有倒计时逻辑：
- 点击"获取验证码"后按钮禁用
- 显示倒计时 "Xs"（如 "60s"）
- 倒计时结束后恢复按钮状态

### 6.3 头像预览更新

新增交互：
- 头像 URL 输入框内容变化时，实时更新头像预览
- 如果 URL 无效或加载失败，显示默认头像

## 7. 文件变更清单

### 7.1 需要修改的文件

| 文件 | 变更类型 | 说明 |
|------|----------|------|
| `ui/register.ui` | 修改 | 重新设计 UI 布局 |
| `src/register.cpp` | 修改 | 更新头像预览逻辑 |
| `src/register.h` | 可选修改 | 如需要添加头像预览相关方法 |

### 7.2 资源文件

- 使用现有资源 `:/assets/user.png` 作为默认头像
- 如需添加新的分组图标，更新 `res/register.qrc`

## 8. 测试检查项

实施完成后需验证：

- [ ] 所有 8 个输入字段正常显示和工作
- [ ] 分组标签视觉呈现正确
- [ ] 验证码按钮内嵌布局正确
- [ ] 头像预览区域显示默认头像
- [ ] 头像 URL 输入时预览实时更新
- [ ] 密码输入框点击时短暂显示
- [ ] 验证码倒计时功能正常
- [ ] 注册按钮样式正确
- [ ] 登录链接可点击跳转
- [ ] 表单验证逻辑正常工作
- [ ] 窗口尺寸为 420x600

## 9. 参考文件

- 现有注册页: `ui/register.ui`
- 现有登录页: `ui/login.ui` (风格参考)
- 主页设计: `ui/home.ui` (样式参考)
- 注册逻辑: `src/register.cpp`, `src/register.h`

---

**设计批准状态**: 待用户批准  
**下一步**: 用户批准后，调用 writing-plans 技能创建实施计划
