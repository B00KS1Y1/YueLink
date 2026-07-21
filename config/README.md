# YueLink configuration

配置模块提供四个相互独立的存储对象：

```cpp
Config::log       // log.json
Config::theme     // theme.json
Config::database  // database.json
Config::application // application.json
```

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
