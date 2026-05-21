# 实施计划: AUTH-001 账号密码登录

**功能**: 用户输入账号密码进行登录验证  
**优先级**: P0 (MVP核心功能)  
**复杂度**: 中等  
**预计耗时**: 4-6小时

---

## 需求分析

### 功能要求
1. 用户输入手机号/用户名和密码
2. 调用后端认证 API 验证凭据
3. 登录成功后获取访问令牌
4. 保存登录状态，进入主界面
5. 登录失败显示错误提示

### 验收标准
- 登录成功后可进入主界面 (Home)
- 登录失败显示明确错误提示
- Token 自动刷新，用户无感知 (AUTH-006)
- 支持记住密码（本地加密存储）

---

## 当前状态

**已有实现:**
- `ui/login.ui` - 登录界面（手机号、用户名、密码输入框）
- `src/login.cpp/h` - 登录窗口类，但 `onLogin()` 只有 `// todo login action`
- 登录后跳转到 `Home` 窗口的逻辑已存在

**缺失实现:**
- HTTP 客户端模块
- 认证服务 (AuthService)
- 后端 API 调用
- 登录状态管理

---

## 实施步骤

### 步骤 1: 创建网络服务基础模块

**文件**: `src/services/http_client.h`, `src/services/http_client.cpp`

**功能:**
- 封装 Qt QNetworkAccessManager
- 提供 POST/GET 同步/异步请求
- JSON 请求体构造和响应解析
- 错误处理

**代码模板:**
```cpp
class HttpClient : public QObject {
    Q_OBJECT
public:
    static HttpClient& instance();
    
    // 异步 POST 请求
    void post(const QString &url, const QJsonObject &body, 
              std::function<void(const QJsonObject &, bool)> callback);
    
    // 同步 POST 请求
    QJsonObject postSync(const QString &url, const QJsonObject &body, bool &ok);
    
private:
    QNetworkAccessManager *manager_;
};
```

**CMakeLists.txt 修改:**
```cmake
# 添加 Qt Network 模块
find_package(Qt6 COMPONENTS Network REQUIRED)
target_link_libraries(MeetEx PRIVATE Qt6::Network)

# 添加服务层文件
set(SERVICE_SOURCES
    src/services/http_client.cpp
    src/services/auth_service.cpp
)
```

---

### 步骤 2: 创建认证服务

**文件**: `src/services/auth_service.h`, `src/services/auth_service.cpp`

**功能:**
- 登录认证接口
- Token 管理和刷新
- 用户资料缓存
- 登录状态持久化

**数据结构:**
```cpp
struct UserProfile {
    QString userId;
    QString displayName;
    QString phone;
    QString email;
    QString avatarUrl;
};

struct AuthToken {
    QString accessToken;
    QString refreshToken;
    QDateTime expiresAt;
};
```

**接口设计:**
```cpp
class AuthService : public QObject {
    Q_OBJECT
public:
    static AuthService& instance();
    
    // 登录
    void login(const QString &account, const QString &password);
    void loginWithPhone(const QString &phone, const QString &code);
    
    // 登出
    void logout();
    
    // 状态查询
    bool isLoggedIn() const;
    QString getAccessToken() const;
    UserProfile getCurrentUser() const;
    
signals:
    void sigLoginSuccess(const UserProfile &profile);
    void sigLoginFailed(const QString &error);
    void sigTokenRefreshed(const QString &newToken);
    
private:
    void saveAuthData(const AuthToken &token, const UserProfile &profile);
    void loadAuthData();
    void clearAuthData();
    
    AuthToken currentToken_;
    UserProfile currentUser_;
    bool isLoggedIn_ = false;
};
```

---

### 步骤 3: 修改登录窗口

**文件**: `src/login.cpp`, `src/login.h`

**修改内容:**

1. **在 `onLogin()` 中调用认证服务:**
```cpp
void Login::onLogin() {
    QString account = ui->usernameTextEdit->toPlainText().trimmed();
    QString password = ui->passwordEextEdit->toPlainText();
    
    // 验证输入
    if (account.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入账号和密码");
        return;
    }
    
    // 禁用登录按钮，显示加载状态
    ui->loginBtn->setEnabled(false);
    ui->loginBtn->setText("登录中...");
    
    // 连接认证服务信号
    connect(&AuthService::instance(), &AuthService::sigLoginSuccess,
            this, &Login::onLoginSuccess);
    connect(&AuthService::instance(), &AuthService::sigLoginFailed,
            this, &Login::onLoginFailed);
    
    // 调用登录
    AuthService::instance().login(account, password);
}
```

2. **添加登录成功/失败处理:**
```cpp
void Login::onLoginSuccess(const UserProfile &profile) {
    login_state = true;
    qInfo() << "Login success:" << profile.displayName;
    close();
}

void Login::onLoginFailed(const QString &error) {
    ui->loginBtn->setEnabled(true);
    ui->loginBtn->setText("登录");
    QMessageBox::critical(this, "登录失败", error);
}
```

---

### 步骤 4: 配置后端 API 地址

**文件**: `meetingengine/meeting_def.h`

**添加:**
```cpp
// 后端 API 配置
#define API_BASE_URL "https://api.meetex.example.com"
#define API_LOGIN_ENDPOINT "/v1/auth/login"
#define API_REFRESH_TOKEN_ENDPOINT "/v1/auth/refresh"
```

---

### 步骤 5: 密码输入框改为密码模式

**文件**: `ui/login.ui`

**修改:**
将 `passwordEextEdit` 从 `QTextEdit` 改为 `QLineEdit`，并设置密码模式：
```xml
<widget class="QLineEdit" name="passwordLineEdit">
    <property name="echoMode">
        <enum>QLineEdit::Password</enum>
    </property>
</widget>
```

---

## 测试计划

### 单元测试

**文件**: `tests/test_auth_service.cpp`

```cpp
void TestAuthService::testLoginSuccess() {
    QSignalSpy spy(&AuthService::instance(), &AuthService::sigLoginSuccess);
    AuthService::instance().login("testuser", "testpass");
    QVERIFY(spy.wait(5000));
    QVERIFY(AuthService::instance().isLoggedIn());
}

void TestAuthService::testLoginFailure() {
    QSignalSpy spy(&AuthService::instance(), &AuthService::sigLoginFailed);
    AuthService::instance().login("testuser", "wrongpass");
    QVERIFY(spy.wait(5000));
    QVERIFY(!AuthService::instance().isLoggedIn());
}
```

### 集成测试

1. 正常登录流程测试
2. 错误密码处理测试
3. 网络异常处理测试
4. 登录后状态保持测试

---

## 风险与注意事项

| 风险 | 可能性 | 缓解措施 |
|------|--------|----------|
| 后端 API 未就绪 | 高 | 先实现模拟模式，使用固定数据测试 |
| 密码明文传输 | 中 | 使用 HTTPS，密码做哈希处理 |
| Token 存储安全 | 中 | 使用 QKeychain 或 Windows Credential |
| 网络超时 | 中 | 设置合理的超时时间，提供重试机制 |

---

## 文件清单

| 文件 | 操作 | 说明 |
|------|------|------|
| `src/services/http_client.h` | 创建 | HTTP 客户端封装 |
| `src/services/http_client.cpp` | 创建 | HTTP 实现 |
| `src/services/auth_service.h` | 创建 | 认证服务接口 |
| `src/services/auth_service.cpp` | 创建 | 认证服务实现 |
| `src/login.cpp` | 修改 | 集成认证服务 |
| `src/login.h` | 修改 | 添加槽函数声明 |
| `ui/login.ui` | 修改 | 密码输入框改为密码模式 |
| `meetingengine/meeting_def.h` | 修改 | 添加 API 配置 |
| `CMakeLists.txt` | 修改 | 添加 Network 模块和服务文件 |
| `tests/test_auth_service.cpp` | 创建 | 单元测试 |

---

## 验证步骤

1. 构建项目，确保无编译错误
2. 运行应用，显示登录窗口
3. 输入测试账号密码，点击登录
4. 验证登录成功跳转到主界面
5. 输入错误密码，验证错误提示
6. 检查 Token 是否正确保存

---

## 后续优化 (可选)

- 添加"记住密码"复选框
- 添加加载动画
- 支持验证码登录 (AUTH-002)
- 添加自动登录功能
