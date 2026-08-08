# YueLink configuration

配置模块为每种配置维护独立 JSON 文件，并通过 nlohmann 的
`NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT` 自动完成结构与 JSON 的映射。
所有顶层配置继承 `ConfigBase<自身类型>`，统一获得默认值、schema 版本、规范化、
校验和迁移扩展点。
业务代码只使用 `configapi.h` 提供的统一模板接口，不直接访问存储对象和文件。

## 内置配置

| 配置类型 | 稳定键名 | 文件 |
| --- | --- | --- |
| `IdentityConfig` | `identity` | `identity.json` |
| `ApplicationConfig` | `application` | `application.json` |
| `ThemeConfig` | `theme` | `theme.json` |
| `LogConfig` | `log` | `log.json` |
| `DatabaseConfig` | `database` | `database.json` |

运行时配置默认保存在可执行文件同级的 `system/configs`。目录仍可通过
`YUELINK_CONFIG_DIR` 覆盖；相对覆盖路径以 system 目录为基准。

每个 JSON 文件使用扁平字段，并包含配置模块保留的版本字段：

```json
{
    "_schema_version": 1,
    "animations_enabled": true,
    "mode": "dark",
    "navigation_mode": "compact",
    "primary_color": "#4F7CFF"
}
```

没有 `_schema_version` 的旧配置按版本 0 加载，因此现有配置可以继续使用。
高于程序支持版本的文件不会被覆盖。

## 初始化

应用程序启动时统一加载所有已注册配置：

```cpp
#include "infrastructure/config/configapi.h"

const Config::Result result = Config::initialize();
if (!result)
{
    qWarning().noquote() << result.errorMessage;
}
```

单个配置加载失败不会阻止其他配置加载。失败配置保留其默认内存值，原 JSON
文件不会被覆盖。

## 读取

`get<T>()` 返回线程安全的配置副本：

```cpp
const Config::ThemeConfig theme = Config::get<Config::ThemeConfig>();
```

需要检测并发变化时可读取带修订号的快照：

```cpp
const Config::Snapshot<Config::ThemeConfig> snapshot =
    Config::snapshot<Config::ThemeConfig>();
```

## 修改

推荐使用 `update<T>()` 修改部分字段：

```cpp
const Config::Result result = Config::update<Config::ThemeConfig>(
    [](Config::ThemeConfig &theme) {
        theme.mode = "system";
        theme.animations_enabled = false;
    });
```

配置模块依次完成副本修改、规范化、校验、`QSaveFile` 原子写入和内存发布。
任何步骤失败时，原文件和当前内存配置均保持不变。修改器没有产生变化时不会写盘。
修改期间如果同一配置已被其他线程更新，操作会返回 `ErrorCode::Conflict`，调用方可基于
最新配置重试，旧快照不会覆盖新值。

整体替换配置使用 `set<T>()`：

```cpp
Config::ThemeConfig theme = Config::get<Config::ThemeConfig>();
theme.mode = "light";
const Config::Result result = Config::set(std::move(theme));
```

`set<T>()` 即使配置值未变化也会确保文件已保存。不要再使用分离的
`set()`/`save()` 调用。

## 重新加载和重置

```cpp
const Config::Result reloadResult = Config::reload<Config::ThemeConfig>();
const Config::Result resetResult = Config::reset<Config::ThemeConfig>();
```

也可以通过 `Config::reloadAll()` 重新加载所有配置。成功发布新值后，
`ConfigManager::configChanged(configKey, revision)` 会发出统一变更通知。

## 添加配置

新增配置时执行以下步骤：

1. 新建独立配置头文件并继承 `ConfigBase<自身类型>`；
2. 在配置类型中声明稳定 `Key`、`FileName` 和成员默认值；
3. 使用 `NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT` 声明自动映射；
4. 将类型加入 `configregistry.h` 的 `BuiltInConfigs`；
5. 将头文件加入 `src/infrastructure/CMakeLists.txt`；
6. 只有存在特殊规则时才在配置类型中声明并实现 `defaults()`、`normalize()`、`validate()` 或 `migrate()`。

示例：

```cpp
struct NetworkConfig final : ConfigBase<NetworkConfig>
{
    static constexpr auto Key = "network";
    static constexpr auto FileName = "network.json";

    std::uint16_t discovery_port = 45454;
    std::size_t heartbeat_interval_ms = 3000;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(NetworkConfig,
                                                discovery_port,
                                                heartbeat_interval_ms)
```

加入注册表后即可使用稳定接口：

```cpp
const NetworkConfig network = Config::get<NetworkConfig>();
```

## 命名和兼容规范

- 配置类型使用 PascalCase，并以 `Config` 结尾；
- C++ 成员和 JSON 键保持 snake_case，以兼容现有文件；
- JSON 文件名和稳定键名使用小写单数形式；
- 数值单位写入字段名，例如 `_ms`、`_seconds`；
- 普通新增字段依靠 `WITH_DEFAULT` 兼容旧文件；
- 默认 schema 版本继承自 `ConfigBase`；需要升级时在派生配置中重新声明 `SchemaVersion`；
- 字段改名或类型变化时增加 schema 版本并实现迁移；
- 配置文件彼此独立，原子性仅覆盖单个 JSON 文件；跨文件更新必须分别处理结果。
