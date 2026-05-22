# MeetEx 服务端部署文档

## 目录

1. [系统概述](#系统概述)
2. [部署架构](#部署架构)
3. [环境要求](#环境要求)
4. [部署步骤](#部署步骤)
   - 4.1 [meetex-srv 媒体服务器](#41-meetex-srv-媒体服务器)
   - 4.2 [meetex-page Web前端](#42-meetex-page-web前端)
   - 4.3 [meetex-nginx 代理服务器](#43-meetex-nginx-代理服务器)
5. [服务管理](#服务管理)
6. [模拟会议用户](#模拟会议用户)

---

## 系统概述

MeetEx 是一款基于 LiveKit 的实时音视频通信系统，包含三个核心组件：

| 组件 | 功能 | 默认端口 |
|------|------|----------|
| meetex-srv | LiveKit 媒体服务器 (SFU) | 7880 (WS), 7881 (TCP), 50000-60000 (UDP) |
| meetex-page | Next.js Web 应用 | 3000 |
| meetex-nginx | Nginx 反向代理 | 443 (HTTPS), 8443 (WSS) |

---

## 部署架构

```
                    ┌─────────────────┐
    User            │   Nginx         │
    ───────►        │   (443/8443)    │
                    └────────┬────────┘
                             │
           ┌─────────────────┼─────────────────┐
           ▼                 ▼                 ▼
    ┌────────────┐    ┌────────────┐    ┌────────────┐
    │ Web App    │    │ WebSocket  │    │  TURN      │
    │ (3000)     │    │ (7880)     │    │  (3478)    │
    └────────────┘    └─────┬──────┘    └────────────┘
                            │
                    ┌───────┴───────┐
                    │  LiveKit SFU   │
                    └────────────────┘
```

---

## 环境要求

### 系统要求

- **操作系统**: Ubuntu 20.04+ / Debian 11+ / CentOS 8+
- **内存**: 最低 2GB，推荐 4GB+
- **网络**: 开放端口 443, 8443, 7880, 7881, 3478, 5349, 50000-60000
- **域名**: 已解析到服务器的域名 (如 www.exrapid.cn)

### 软件依赖

| 软件 | 版本 | 用途 |
|------|------|------|
| Node.js | v24.15.0 | Web 应用运行时 |
| npm | 11.12.1 | 包管理 |
| pnpm | 11.1.2 | 项目构建 |
| pm2 | 7.0.1 | 进程管理 |
| Nginx | 1.18+ | 反向代理 |
| FFmpeg | 4.4+ | 模拟用户推流 |

---

## 部署步骤

### 4.1 meetex-srv 媒体服务器

#### 4.1.1 安装 LiveKit Server

```bash
# 安装 livekit-server 服务端程序
curl -sSL https://get.livekit.io | bash

# 安装 lk CLI 客户端工具
curl -sSL https://get.livekit.io/cli | bash
```

#### 4.1.2 生成 API 密钥

```bash
# 生成新的密钥对（如需自定义）
livekit-server generate-keys
```

#### 4.1.3 配置证书

创建证书目录并放置 SSL 证书：

```bash
sudo mkdir -p /etc/livekit/cert
sudo cp server.crt /etc/livekit/cert/
sudo cp server.key /etc/livekit/cert/
sudo chmod 600 /etc/livekit/cert/server.key
```

#### 4.1.4 创建配置文件

**路径**: `/etc/livekit/livekit.yaml`

```yaml
port: 7880
keys:
  APIbZ3hZTiHdPXN: tAkbkXLkKsAB2e1PCKj3t9EfSMJ8zWcps0phfXBFRP7B
rtc:
  tcp_port: 7881
  port_range_start: 50000
  port_range_end: 60000
  use_external_ip: true
  allow_tcp_fallback: true
turn:
  enabled: true
  domain: "www.exrapid.cn"
  tls_port: 5349
  udp_port: 3478
  cert_file: "/etc/livekit/cert/server.crt"
  key_file: "/etc/livekit/cert/server.key"
```

**配置说明**:
- `port`: WebSocket 服务端口
- `keys`: API 密钥对，用于客户端认证
- `rtc`: WebRTC 配置，包括 UDP 端口范围和 TCP 回退
- `turn`: TURN 服务器配置，用于 NAT 穿透

#### 4.1.5 创建日志目录

```bash
sudo mkdir -p /var/log/livekit
```

#### 4.1.6 配置 Systemd 服务

**路径**: `/etc/systemd/system/livekit-server.service`

```ini
[Unit]
Description=LiveKit Server
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
User=root
ExecStart=/usr/local/bin/livekit-server --config /etc/livekit/livekit.yaml
StandardOutput=append:/var/log/livekit/livekit.log
StandardError=append:/var/log/livekit/livekit.log
Restart=always
RestartSec=5
LimitNOFILE=65535

[Install]
WantedBy=multi-user.target
```

#### 4.1.7 启动服务

```bash
sudo systemctl daemon-reload
sudo systemctl enable livekit-server
sudo systemctl start livekit-server
sudo systemctl status livekit-server
```

---

### 4.2 meetex-page Web前端

#### 4.2.1 安装 Node.js 依赖

```bash
# 安装 pnpm
npm install -g pnpm

# 安装 pm2
npm install -g pm2
```

#### 4.2.2 获取源码

```bash
git clone https://github.com/livekit-examples/meet.git /opt/code/meet
cd /opt/code/meet
```

#### 4.2.3 安装项目依赖

```bash
pnpm install
```

#### 4.2.4 配置环境变量

**路径**: `/opt/code/meetex/.env.production`

```bash
# REQUIRED SETTINGS
LIVEKIT_API_KEY=APIbZ3hZTiHdPXN
LIVEKIT_API_SECRET=tAkbkXLkKsAB2e1PCKj3t9EfSMJ8zWcps0phfXBFRP7B
LIVEKIT_URL=wss://www.exrapid.cn:8443

# OPTIONAL SETTINGS
# S3_KEY_ID=
# S3_KEY_SECRET=
# S3_ENDPOINT=
# S3_BUCKET=
# S3_REGION=
# NEXT_PUBLIC_SHOW_SETTINGS_MENU=true
# NEXT_PUBLIC_LK_RECORD_ENDPOINT=/api/record
```

#### 4.2.5 配置 Next.js 输出模式

确保 `next.config.js` 包含：

```javascript
/** @type {import('next').NextConfig} */
const nextConfig = {
  output: 'standalone',
};

module.exports = nextConfig;
```

#### 4.2.6 构建项目

```bash
pnpm build
```

#### 4.2.7 准备部署包

```bash
# 复制静态资源
cp -rf public/ .next/standalone/
cp -rf .next/static/ .next/standalone/.next/

# 复制环境配置
cp -rf .env.production .next/standalone/.env.production

# 打包
tar -C .next --transform='s,^standalone,meetex,' -zcvf /tmp/meetex.tar.gz standalone
```

#### 4.2.8 部署到服务器

```bash
# 解压部署包
sudo mkdir -p /opt/code/meetex
sudo tar -xzf /tmp/meetex.tar.gz -C /opt/code/
sudo chown -R root:root /opt/code/meetex
```

#### 4.2.9 配置 PM2

**路径**: `/opt/code/meetex/ecosystem.config.js`

```javascript
module.exports = {
  apps: [
    {
      name: "meetex",
      script: "server.js",
      instances: 1,
      exec_mode: "cluster",
      out_file: "/var/log/meetex/meetex.log",
      error_file: "/var/log/meetex/error.log",
      log_date_format: "YYYY-MM-DD HH:mm Z",
    },
  ],
};
```

#### 4.2.10 创建日志目录

```bash
sudo mkdir -p /var/log/meetex
```

#### 4.2.11 配置 Systemd 服务

**路径**: `/etc/systemd/system/meetex.service`

```ini
[Unit]
Description=MeetEx WebPage
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
User=root
WorkingDirectory=/opt/code/meetex
ExecStart=/usr/local/bin/pm2 start ecosystem.config.js --no-daemon
StandardOutput=append:/var/log/meetex/meetex.log
StandardError=append:/var/log/meetex/error.log
Restart=always
RestartSec=5
LimitNOFILE=65535

[Install]
WantedBy=multi-user.target
```

#### 4.2.12 启动服务

```bash
sudo systemctl daemon-reload
sudo systemctl enable meetex
sudo systemctl start meetex
sudo systemctl status meetex
```

---

### 4.3 meetex-nginx 代理服务器

#### 4.3.1 安装 Nginx

```bash
sudo apt update
sudo apt install -y nginx
```

#### 4.3.2 创建配置文件

**路径**: `/etc/nginx/sites-available/livekit`

```nginx
server {
    listen 443 ssl http2;
    server_name www.exrapid.cn;

    ssl_certificate     /etc/livekit/cert/server.crt;
    ssl_certificate_key /etc/livekit/cert/server.key;
    ssl_protocols       TLSv1.2 TLSv1.3;

    # 网站（Next.js / 静态站等）
    location / {
        proxy_pass http://127.0.0.1:3000;
        proxy_http_version 1.1;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
    }
}

server {
    listen 8443 ssl http2;
    server_name www.exrapid.cn;

    ssl_certificate     /etc/livekit/cert/server.crt;
    ssl_certificate_key /etc/livekit/cert/server.key;
    ssl_protocols       TLSv1.2 TLSv1.3;

    # WSS（LiveKit Server）
    location / {
        proxy_pass http://127.0.0.1:7880;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
        proxy_read_timeout 86400s;
        proxy_send_timeout 86400s;
    }
}
```

#### 4.3.3 启用配置

```bash
# 创建软链接
sudo ln -sf /etc/nginx/sites-available/livekit /etc/nginx/sites-enabled/livekit

# 测试配置
sudo nginx -t

# 重启 Nginx
sudo systemctl restart nginx
sudo systemctl enable nginx
```

#### 4.3.4 防火墙配置

```bash
# 开放必要端口
sudo ufw allow 22/tcp
sudo ufw allow 80/tcp
sudo ufw allow 443/tcp
sudo ufw allow 8443/tcp
sudo ufw allow 7880/tcp
sudo ufw allow 7881/tcp
sudo ufw allow 3478/udp
sudo ufw allow 5349/tcp
sudo ufw allow 50000:60000/udp
sudo ufw --force enable
```

---

## 服务管理

### 查看服务状态

```bash
# LiveKit 服务
sudo systemctl status livekit-server
sudo journalctl -u livekit-server -f

# Web 服务
sudo systemctl status meetex
sudo journalctl -u meetex -f

# Nginx
sudo systemctl status nginx
sudo tail -f /var/log/nginx/error.log
```

### PM2 管理

```bash
# 查看应用状态
pm2 status

# 查看日志
pm2 logs meetex

# 重启应用
pm2 restart meetex

# 停止应用
pm2 stop meetex
```

### 端口检查

```bash
# 检查端口占用
sudo ss -tlnp | grep -E '7880|3000|443|8443'

# 检查 LiveKit 健康状态
curl http://localhost:7880
```

---

## 模拟会议用户

用于测试的脚本，使用 FFmpeg 向会议室推送模拟视频流。

### 前置条件

```bash
# 安装 FFmpeg
sudo apt install -y ffmpeg

# 准备测试视频
sudo mkdir -p /opt/data
sudo cp videoplayback1.mp4 videoplayback2.mp4 videoplayback3.mp4 videoplayback4.mp4 /opt/data/
```

### 脚本位置

**路径**: `/opt/code/meetex/meetexrapid.sh`

```bash
#!/bin/bash

if pgrep -f ffmpeg >/dev/null 2>&1; then
  echo "Found ffmpeg process(es), killing..."
  pkill -9 -f ffmpeg
else
  echo "No ffmpeg process found."
fi

if pgrep -f 'lk room' >/dev/null 2>&1; then
  echo "Found lk room process(es), killing..."
  pkill -9 -f 'lk room'
else
  echo "No lk room process found."
fi

rm -rf /tmp/videoplayback*_video.sock
rm -rf /tmp/videoplayback*_audio.sock
rm -rf /tmp/lk_*.log

# 启动 4 个模拟用户 (Optimus, Bumblebee, Megatron, Starscream)
for i in 1 2 3 4; do
  nohup ffmpeg -stream_loop -1 -i /opt/data/videoplayback${i}.mp4 \
    -c:v libx264 -bsf:v h264_mp4toannexb -r 10 -b:v 300K -crf 32 \
    -profile:v baseline -pix_fmt yuv420p -x264-params keyint=120 \
    -max_delay 0 -bf 0 -listen 1 -f h264 unix:/tmp/videoplayback${i}_video.sock \
    -c:a libopus -page_duration 20000 -vn -listen 1 -f opus unix:/tmp/videoplayback${i}_audio.sock \
    > /dev/null 2>&1 &
  echo "Started ffmpeg for videoplayback${i}.mp4"
  sleep 5

  nohup lk room join --url ws://127.0.0.1:7880 \
    --api-key APIbZ3hZTiHdPXN \
    --api-secret tAkbkXLkKsAB2e1PCKj3t9EfSMJ8zWcps0phfXBFRP7B \
    --identity "User${i}" \
    --publish h264:///tmp/videoplayback${i}_video.sock \
    --publish opus:///tmp/videoplayback${i}_audio.sock \
    meetex > /tmp/lk_User${i}.log 2>&1 &
  echo "Started lk room join for User${i}"
  sleep 5
done
```

### 运行脚本

```bash
chmod +x /opt/code/meetex/meetexrapid.sh
sudo /opt/code/meetex/meetexrapid.sh
```

### 停止模拟用户

```bash
pkill -9 -f ffmpeg
pkill -9 -f 'lk room'
rm -rf /tmp/videoplayback*_video.sock /tmp/videoplayback*_audio.sock /tmp/lk_*.log
```

---

## 验证部署

1. **访问 Web 应用**: https://www.exrapid.cn
2. **创建会议室**: 输入房间名并加入
3. **检查媒体流**: 确认音视频正常传输
4. **测试模拟用户**: 运行 `meetexrapid.sh` 验证多人会议
