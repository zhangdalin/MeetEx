# MeetEx API Server

MeetEx 后端验证服务器，提供用户认证和会议管理 API。

## 功能特性

- 账号密码登录（SHA256 + bcrypt 双重加密）
- 手机验证码登录
- JWT Token 认证（Access Token + Refresh Token）
- Token 自动刷新
- SQLite 数据库存储（可迁移至 PostgreSQL）

## API 端点

| 端点 | 方法 | 描述 |
|------|------|------|
| `/` | GET | 服务状态 |
| `/health` | GET | 健康检查 |
| `/v1/auth/login` | POST | 账号密码登录 |
| `/v1/auth/login/phone` | POST | 手机验证码登录 |
| `/v1/auth/refresh` | POST | Token 刷新 |
| `/docs` | GET | Swagger API 文档 |
| `/redoc` | GET | ReDoc API 文档 |

## 快速开始

### 1. 环境要求

- Python 3.10+
- pip

### 2. 安装依赖

```bash
cd meetex-api
pip install -r requirements.txt
```

### 3. 启动服务

**开发模式（热重载）:**
```bash
./start.sh dev
```

**生产模式:**
```bash
./start.sh prod
```

或使用 Python 直接启动:
```bash
# 开发
uvicorn app.main:app --reload --host 0.0.0.0 --port 8000

# 生产
gunicorn -w 4 -k uvicorn.workers.UvicornWorker --bind 0.0.0.0:8000 app.main:app
```

### 4. 访问 API

- API 地址: http://localhost:8000
- 文档地址: http://localhost:8000/docs

## 生产部署

### 使用 systemd 服务

1. **创建用户和目录:**
```bash
sudo mkdir -p /opt/code/meetex-api
sudo cp -r * /opt/code/meetex-api/
```

2. **创建日志目录:**
```bash
sudo mkdir -p /var/log/meetex-api
sudo chown -R meetex:meetex /var/log/meetex-api
```

3. **安装依赖:**
```bash
cd /opt/meetex-api
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
```

4. **配置环境变量:**
编辑 `meetex-api.service`，修改:
- `JWT_SECRET_KEY`: 生成强密钥
- `CORS_ORIGINS`: 配置允许的域名

5. **安装服务:**
```bash
sudo cp meetex-api.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable meetex-api
sudo systemctl start meetex-api
```

6. **管理服务:**
```bash
# 查看状态
sudo systemctl status meetex-api

# 查看日志
sudo journalctl -u meetex-api -f

# 重启服务
sudo systemctl restart meetex-api
```

### Nginx 反向代理

添加配置到 `3.meetex-nginx/`:

```nginx
upstream meetex_api {
    server 127.0.0.1:8000;
}

server {
    listen 443 ssl;
    server_name api.meetex.exrapid.cn;

    ssl_certificate /etc/nginx/cert/server.crt;
    ssl_certificate_key /etc/nginx/cert/server.key;

    location / {
        proxy_pass http://meetex_api;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
        proxy_buffering off;
        proxy_request_buffering off;
    }
}
```

## 环境变量

| 变量 | 默认值 | 说明 |
|------|--------|------|
| `DATABASE_URL` | `sqlite:///./meetex.db` | 数据库连接 |
| `JWT_SECRET_KEY` | 自动生成 | JWT 签名密钥 |
| `JWT_ACCESS_TOKEN_EXPIRE_MINUTES` | `60` | Access Token 有效期(分钟) |
| `JWT_REFRESH_TOKEN_EXPIRE_DAYS` | `7` | Refresh Token 有效期(天) |
| `CORS_ORIGINS` | `*` | 允许的跨域来源，逗号分隔 |
| `DEBUG` | `False` | 调试模式 |

## 客户端集成

客户端 `meetingengine/meeting_def.h` 配置:

```cpp
#define API_BASE_URL "https://api.meetex.exrapid.cn"
#define API_LOGIN_ENDPOINT "/v1/auth/login"
#define API_REFRESH_TOKEN_ENDPOINT "/v1/auth/refresh"
```

## 数据库迁移

SQLite 数据库文件位于 `./meetex.db`。如需迁移到其他数据库:

1. 修改 `DATABASE_URL` 环境变量
2. 重启服务，表结构会自动创建

示例 PostgreSQL:
```bash
DATABASE_URL="postgresql+asyncpg://user:password@localhost/meetex"
```

## 开发测试

### 创建测试用户

```bash
curl -X POST http://localhost:8000/v1/auth/register \
  -H "Content-Type: application/json" \
  -d '{
    "account": "test",
    "password": "'$(echo -n "password123" | sha256sum | cut -d' ' -f1)'",
    "display_name": "Test User"
  }'
```

### 登录测试

```bash
curl -X POST http://localhost:8000/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{
    "account": "test",
    "password": "'$(echo -n "password123" | sha256sum | cut -d' ' -f1)'"
  }'
```

### 手机登录测试

```bash
curl -X POST http://localhost:8000/v1/auth/login/phone \
  -H "Content-Type: application/json" \
  -d '{
    "phone": "13800138000",
    "code": "123456"
  }'
```

## 项目结构

```
meetex-api/
├── app/
│   ├── __init__.py
│   ├── main.py          # FastAPI 入口
│   ├── config.py        # 配置管理
│   ├── database.py      # 数据库连接
│   ├── models.py        # SQLAlchemy 模型
│   ├── schemas.py       # Pydantic 模型
│   ├── auth_utils.py    # JWT 和密码工具
│   └── routers/
│       ├── __init__.py
│       └── auth.py      # 认证路由
├── requirements.txt
├── start.sh             # 启动脚本
├── meetex-api.service   # systemd 配置
└── README.md
```

## 注意事项

1. **生产环境必须修改 `JWT_SECRET_KEY`**，使用随机生成的强密钥
2. **密码安全**: 客户端发送 SHA256 哈希，服务端使用 bcrypt 再次哈希存储
3. **CORS**: 生产环境应配置具体的域名，不要使用 `*`
4. **数据库**: SQLite 适合开发和轻量使用，生产建议使用 PostgreSQL
