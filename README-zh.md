N2N VPN 管理平台使用指南
系统架构
+----------------+        +---------------+        +-------------+
|  N2N客户端GUI  | <----> | PostgreSQL DB | <----> | N2N Supernode|
+----------------+        +---------------+        +-------------+
一、环境准备
1. 部署N2N Supernode（必需）
# 安装n2n
sudo apt install n2n

# 创建supernode服务（示例配置）
sudo nano /etc/systemd/system/n2n-supernode.service

[Unit]
Description=N2N Supernode
After=network.target

[Service]
ExecStart=/usr/sbin/supernode -l 7654 -f
Restart=always

[Install]
WantedBy=multi-user.target

# 启动服务
sudo systemctl daemon-reload
sudo systemctl start n2n-supernode
sudo systemctl enable n2n-supernode

# 防火墙设置
sudo ufw allow 7654/udp

2. 部署PostgreSQL数据库（可选）
-- 创建数据库
CREATE DATABASE n2n WITH ENCODING 'UTF8';

-- 创建服务器表
CREATE TABLE servers (
    id SERIAL PRIMARY KEY,
    node_name VARCHAR(50) NOT NULL UNIQUE,
    ip_address INET NOT NULL,
    created_at TIMESTAMP DEFAULT NOW()
);

-- 创建用户表
CREATE TABLE users (
    user_id SERIAL PRIMARY KEY,
    username VARCHAR(30) NOT NULL UNIQUE,
    password VARCHAR(100) NOT NULL,
    last_login TIMESTAMP
);

二、软件配置
1. 配置文件模板 (config.ini)
[Database]
Host=your.supernode.address  ; 对应RemoteDatabaseHandler的host配置
Port=5432
DatabaseName=n2n
UserName=postgres
Password=your_secure_password
