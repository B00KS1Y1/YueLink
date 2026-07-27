# YueLink

## 源码结构

- `src/core`：共享业务核心、领域类型与抽象接口。
- `src/infrastructure`：网络、存储、配置、日志与运行时装配。
- `src/gui`：Qt Quick GUI 入口、QML、模型与桌面适配器。
- `src/cli`：基于 `QCoreApplication` 的终端前端。
- `tests`：核心服务与命令解析测试。

YueLink 由共享聊天核心和两个前端组成：

- `YueLink`：Qt Quick/HuskarUI 图形界面。
- `yuelink-cli`：基于 `QCoreApplication` 的行式终端客户端。
- `YueLinkCore`：身份、好友、会话、消息和文件传输编排。
- `YueLinkInfrastructure`：UDP 发现、TCP 传输、SQLite、配置和日志。

两个前端使用相同的身份、配置和数据库。当前同一份配置只允许启动一个实例；
GUI 与 CLI 同时启动时，后启动的进程会退出，避免重复广播同一个设备身份。

## 运行时目录

默认在可执行文件同级创建以下结构，缺失的目录会在启动时自动创建：

```text
system/
├── configs/    # JSON 配置与 identity.ini
├── logs/       # yuelink.log 及轮转日志
└── database/   # yuelink.db、WAL 与 SHM 文件
```

运行锁文件保留在应用数据根目录，不放入数据库目录。相对日志与 SQLite 路径
分别以 `system/logs` 和 `system/database` 为基准，显式绝对路径不受影响。
目录可在启动前通过环境变量覆盖：

- `YUELINK_SYSTEM_DIR`：system 根目录；相对路径以可执行文件目录为基准；
- `YUELINK_CONFIG_DIR`：配置目录；相对路径以 system 根目录为基准；
- `YUELINK_LOG_DIR`：日志目录；相对路径以 system 根目录为基准；
- `YUELINK_DATABASE_DIR`：数据库目录；相对路径以 system 根目录为基准。

每项都支持绝对路径。日志文件名和 SQLite 文件名仍分别通过 `log.json` 与
`database.json` 的 `file_path` 配置。

## CLI

启动终端模式：

```text
yuelink-cli
```

自动化场景可以使用 JSON Lines 输出：

```text
yuelink-cli --jsonl
```

交互命令：

```text
status
peers [online|all]
use <peer-id|序号>
messages [数量]
send <文本>
send-file <路径> [更多路径]
transfers
cancel <transfer-id>
read
profile
profile set <昵称>
config show
quit
```

包含空格的消息和路径可以使用单引号或双引号。

## 运行时边界

`ChatService` 不依赖 QML、窗口状态、桌面通知或文件启动器。GUI 的
`LanChatManager` 负责把领域数据转换为 QML 列表模型，并处理窗口活跃状态、
通知及文件打开。CLI 直接订阅 `ChatService` 事件，不链接 Qt Quick、Qt Widgets
或 HuskarUI。

数据库 schema 版本 2 新增原始文件字节数字段；现有版本 1 数据库会在首次启动时
自动添加该字段，原有的展示文本仍作为旧记录兼容数据保留。

只构建核心与 CLI 时可以关闭 GUI 目标：

```text
-DYUELINK_BUILD_GUI=OFF
```

此模式不查找 Qt Quick、Qt Widgets 或 HuskarUI。
