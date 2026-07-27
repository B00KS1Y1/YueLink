# YueLink configuration

配置模块提供四个相互独立的存储对象：

```cpp
Config::log       // log.json
Config::theme     // theme.json
Config::database  // database.json
Config::application // application.json
```

运行时配置默认保存在可执行文件同级的 `system/configs`：

- `log.json`：日志配置，默认日志文件为 `system/logs/yuelink.log`；
- `theme.json`：主题配置；
- `database.json`：数据库配置，默认 SQLite 文件为 `system/database/yuelink.db`；
- `application.json`：应用配置；
- `identity.ini`：本机设备标识与昵称。

相对日志路径始终以 `system/logs` 为基准，相对 SQLite 路径始终以
`system/database` 为基准；配置中显式填写的绝对路径保持不变。
为兼容已有配置，日志路径开头的 `logs/` 以及 SQLite 路径开头的 `data/`
或 `database/` 会被移除，避免生成重复目录。
system、配置、日志和数据库目录可分别通过 `YUELINK_SYSTEM_DIR`、
`YUELINK_CONFIG_DIR`、`YUELINK_LOG_DIR`、`YUELINK_DATABASE_DIR` 覆盖。
system 相对路径以可执行文件目录为基准，其余相对路径以 system 目录为基准；
所有变量也接受绝对路径。

每个对象都只提供 `get()`、`set()`、`load()`、`save()`。

## 加载

```cpp
const Config::Result result = Config::theme.load();
if (!result)
{
    qWarning().noquote() << result.errorMessage;
}
```

文件不存在时使用该配置类型的默认值。文件无法读取或解析时保留当前内存配置，不生成备份，也不覆盖原文件。

## 读取和修改

`get()` 返回配置副本，`set()` 只更新对应配置的内存值：

```cpp
Config::ThemeConfig theme = Config::theme.get();
theme.mode = "system";
theme.primary_color = "#4F7CFF";

Config::theme.set(std::move(theme));
```

## 保存

每个配置单独保存：

```cpp
const Config::Result result = Config::theme.save();
```

保存使用 `QSaveFile`。写入失败时，原来的 JSON 文件保持不变。

## 命名规范

- 类型使用 PascalCase：`ThemeConfig`、`LogConfig`、`ConfigStore`。
- 对象和方法使用 lowerCamelCase：`theme`、`get()`。
- 配置 DTO 字段与 JSON schema 一致，统一使用 snake_case；序列化由 nlohmann 宏自动生成。
