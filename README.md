# YueLink

YueLink 是一个基于 Qt 6、Qt Quick 与 HuskarUI 的局域网聊天应用。

## 源码结构

- `src/core`：领域类型与基础设施抽象接口；
- `src/application`：聊天生命周期、会话存储和文件传输编排；
- `src/infrastructure`：UDP 发现、TCP 传输、SQLite、JSON 配置和日志；
- `src/gui`：Qt Quick 入口、QML、视图模型与桌面平台适配器。

主要构建目标：

- `YueLink`：Qt Quick/HuskarUI 图形界面；
- `YueLinkCore`：领域类型与抽象接口；
- `YueLinkInfrastructure`：网络、数据库、配置和日志实现；
- `YueLinkApplication`：应用用例协调与状态管理。

## 运行时目录

默认在可执行文件同级创建以下结构，缺失的目录会在启动时自动创建：

```text
system/
├── configs/    # identity.json 及其他 JSON 配置
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
`database.json` 的 `file_path` 配置。本机设备标识与昵称保存在
`identity.json`。

## 架构边界

`ChatCoordinator` 负责应用生命周期、文本消息用例和基础设施编排；
`ConversationStore` 负责好友、会话缓存与持久化；`TransferCoordinator`
负责文件传输状态机。

GUI 通过 `ConversationViewModel` 和 `PeerListViewModel` 向 QML 暴露状态，
`DesktopIntegration` 处理剪贴板、通知和文件启动。`LanChatManager` 仅保留
稳定的 QML 外观 API 与信号转发，不再承载领域规则或平台逻辑。

数据库 schema 版本 2 新增原始文件字节数字段；现有版本 1 数据库会在首次启动时
自动添加该字段，原有的展示文本仍作为旧记录兼容数据保留。

如只需构建库目标，可以关闭 GUI：

```text
-DYUELINK_BUILD_GUI=OFF
```

此模式不查找 Qt Quick、Qt Widgets 或 HuskarUI。
