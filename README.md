# YueLink

YueLink 是一个基于 Qt 6、Qt Quick 与 HuskarUI 的局域网聊天应用。

## 源码结构

```text
src/
├── domain/                         # 领域模型与端口接口
├── application/                    # 聊天用例与状态编排
├── infrastructure/
│   └── config/                     # JSON 配置模型与读写
└── YueLink/                        # QML 模块、桌面适配与程序入口
    └── assets/                     # 应用图标等平台资源
```

目录只表达领域、应用、基础设施和界面四个模块；除配置与静态资源外不再增加
仅用于文件分类的子目录。更细的依赖边界继续由 CMake 目标表达，顶层
`CMakeLists.txt` 只负责依赖发现和模块装配。

主要构建目标：

- `YueLink`：Qt Quick/HuskarUI 图形界面；
- `YueLinkDomain`：领域类型与抽象接口；
- `YueLinkApplication`：应用用例协调与状态管理；
- `YueLinkRuntime`：JSON 配置、日志、路径和运行时初始化；
- `YueLinkNetwork`：局域网发现、消息与文件传输；
- `YueLinkPersistence`：SQLite 会话仓储。

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

`YueLinkDomain` 位于依赖底部，不依赖任何应用实现。`YueLinkNetwork` 与
`YueLinkPersistence` 实现领域层端口，`YueLinkApplication` 负责协调用例，
`YueLink` 表现层在程序入口中装配所有具体实现。

`ChatCoordinator` 负责应用生命周期、文本消息用例和基础设施编排；
`ConversationStore` 负责好友、会话缓存与持久化；`TransferCoordinator`
负责文件传输状态机。

GUI 通过 `ConversationViewModel` 和 `PeerListViewModel` 向 QML 暴露状态，
`DesktopIntegration` 处理剪贴板、通知和文件启动。`LanChatManager` 仅保留
稳定的 QML 外观 API 与信号转发，不再承载领域规则或平台逻辑。

数据库 schema 版本 2 新增原始文件字节数字段；现有版本 1 数据库会在首次启动时
自动添加该字段，原有的展示文本仍作为旧记录兼容数据保留。
