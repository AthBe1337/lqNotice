# lqNotice

检查蓝桥杯官网通知前十条，如果存在获奖名单，则发送邮件通知。

## 构建项目

首先安装依赖库

```bash
sudo apt update
sudo apt install -y build-essential cmake libssl-dev libcurl4-openssl-dev nlohmann-json3-dev
```

克隆仓库

```bash
git clone https://github.com/AthBe1337/lqNotice.git
cd lqNotice
```

运行CMake

```bash
mkdir build
cd build
cmake ..
```

构建项目

```bash
make
```

## 使用方法

在运行目录下创建文件`config/settings.json`

内容如下

```json
{
  "check_interval": 600,   //查询间隔，单位为秒
  "target_url": "https://www.guoxinlanqiao.com/api/news/find?status=1&project=dasai&progid=20&pageno=1&pagesize=10",
  "smtp": {
    "server": "你的smtp服务器",
    "port": 465, //如果不使用ssl则替换为25
    "username": "邮箱用户名",
    "password": "密码",
    "security": "ssl" //不使用ssl则留空
  },
  "trigger_keywords": ["总决赛", "获奖名单", "第十六届"],      //关键词列表，跟据实际情况调整
  "recipients": ["user1@example.com", "user2@example.com"], //收件人列表，如果不使用邮箱则留空
  "server_chan": {
    "enabled": true,
    "uid": "你的UID",
    "sendkey": "你的SendKey"
  }
}

```

### Server酱设置相关

前往[Server酱³ · 极简推送服务](https://sc3.ft07.com/)注册账号以获取uid和sendkey，安装客户端即可接收推送。

