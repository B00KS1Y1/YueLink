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

配置目录为固定路径，不提供用户设置或环境变量覆盖：Debug 构建保存在顶层
`CMakeLists.txt` 所在项目根目录的 `system/configs`；Release、RelWithDebInfo 和
MinSizeRel 构建保存在可执行文件目录的 `system/configs`。

每个 JSON 文件使用扁平字段，并包含配置模块保留的版本字段：

```json
{
    "_schema_version": 1,
    "animations_enabled": true,
    "mode": "dark",
    "primary_color": "#4F7CFF"
}
```

没有 `_schema_version` 的旧配置按版本 0 加载，因此现有配置可以继续使用。
高于程序支持版本的文件不会被覆盖。

## 初始化

应用程序启动时统一加载所有已注册配置：

```cpp
#include "config/configapi.h"
#include "infrastructure/path.h"

const Config::Result result = Config::initialize(Path::configDirectory());
if (!result)
{
    qWarning().noquote() << result.errorMessage;
}
```

单个配置加载失败不会阻止其他配置加载。失败配置保留其默认内存值，原 JSON
文件不会被覆盖。

配置文件不存在时，配置模块会使用对应类型的成员默认值生成新文件。已有 JSON
缺少可选字段或字段规范化后发生变化时，会补入默认值并通过 `QSaveFile` 原子回写；
未知字段会保留。

下载目录、日志文件和 SQLite 文件路径均在配置加载或写入阶段完成规范化与校验。
配置发布后业务模块直接使用最终绝对路径，不再自行回退或重复校验。

身份配置会在同一边界规范化 UUID、展示名称、头像绝对路径和头像颜色；首次启动时
设备标识与展示名称允许为空，由身份初始化流程补全，非空持久化值必须满足相应格式和长度约束。

## 读取

`value<T>()` 返回线程安全的配置副本：

```cpp
const Config::ThemeConfig theme = Config::value<Config::ThemeConfig>();
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
Config::ThemeConfig theme = Config::value<Config::ThemeConfig>();
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
5. 将头文件及可选的同名实现文件加入 `config/CMakeLists.txt`；
6. 只有存在特殊规则时才在配置类型中声明 `defaults()`、`normalize()`、`validate()` 或 `migrate()`，并在同名 `.cpp` 中实现。

注册表会在编译期检查所有配置的 `Key` 和 `FileName` 是否唯一；重复标识会直接触发静态断言。
跨配置复用的规范化与校验辅助逻辑统一放在私有 `configpolicyutils_p.h/.cpp` 中，不集中存放具体配置策略。

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
const NetworkConfig network = Config::value<NetworkConfig>();
```

## 命名和兼容规范

- 配置类型使用 PascalCase，并以 `Config` 结尾；
- C++ 成员和 JSON 键保持 snake_case，以兼容现有文件；
- JSON 文件名和稳定键名使用小写单数形式；
- 数值单位写入字段名，例如 `_ms`、`_seconds`；
- 普通新增字段依靠 `WITH_DEFAULT` 兼容旧文件，并在成功加载后补全回写；
- 默认 schema 版本继承自 `ConfigBase`；需要升级时在派生配置中重新声明 `SchemaVersion`；
- 字段改名或类型变化时增加 schema 版本并实现迁移；
- 配置文件彼此独立，原子性仅覆盖单个 JSON 文件；跨文件更新必须分别处理结果。
