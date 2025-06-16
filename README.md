# lqNotice

通过检查蓝桥杯官网通知前十条，如果存在获奖名单，则发送邮件通知。

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
  "check_interval": 600,
  "target_url": "https://www.guoxinlanqiao.com/api/news/find?status=1&project=dasai&progid=20&pageno=1&pagesize=10",
  "smtp": {
    "server": "你的smtp服务器",
    "port": 465, //如果不使用ssl则替换为25
    "username": "邮箱用户名",
    "password": "密码",
    "security": "ssl" //不使用ssl则留空
  },
  "trigger_keywords": ["总决赛", "获奖名单", "第十六届"],      //关键词列表，跟据实际情况调整
  "recipients": ["user1@example.com", "user2@example.com"] //收件人列表，跟据实际情况调整
}

```

程序会每隔十分钟进行一次检查，当成功检测到时，发送邮件并退出。