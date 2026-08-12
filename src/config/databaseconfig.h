/**
 * @file databaseconfig.h
 * @brief 定义数据库配置及其 JSON 自动映射。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-08
 */

#ifndef DATABASECONFIG_H
#define DATABASECONFIG_H

#include "configbase.h"

#include <nlohmann/json.hpp>

#include <cstddef>
#include <cstdint>
#include <string>

namespace Config
{

/**
 * @brief 保存 SQLite 连接、并发访问和持久化策略。
 */
struct SqliteConfig
{
    /// 数据库文件路径；相对路径按应用数据库目录解析。
    std::string file_path = "yuelink.db";
    /// 连接池大小，必须大于零。
    std::size_t pool_size = 1;
    /// 数据库锁定时的等待上限，单位为毫秒，取值范围为 1 至 600000。
    std::size_t busy_timeout_ms = 5000;
    /// 是否启用 WAL 日志模式。
    bool wal_enabled = true;
    /// 是否为每个连接启用外键约束。
    bool foreign_keys_enabled = true;
    /// SQLite synchronous 模式，可取 @c off、@c normal、@c full 或 @c extra。
    std::string synchronous = "normal";
};

/**
 * @brief 保存 MySQL 连接参数。
 *
 * 当前版本仍会序列化这些字段以保持配置格式稳定，但运行时只支持 SQLite 驱动。
 */
struct MySqlConfig
{
    /// 数据库服务器主机名或 IP 地址。
    std::string host = "127.0.0.1";
    /// 数据库服务器 TCP 端口。
    std::uint16_t port = 3306;
    /// 要连接的数据库名称。
    std::string database = "yuelink";
    /// 登录用户名。
    std::string username = "root";
    /// 登录密码；该值会以明文写入配置 JSON，不承担密钥存储职责。
    std::string password;
    /// 建立连接的超时上限，单位为秒。
    std::size_t connect_timeout_seconds = 5;
    /// 预期连接池大小。
    std::size_t pool_size = 4;
    /// 连接字符集名称。
    std::string charset = "utf8mb4";
};

/**
 * @brief 聚合数据库驱动选择及其驱动专用设置。
 */
struct DatabaseConfig final : ConfigBase<DatabaseConfig>
{
    /// 配置注册表、变更信号使用的稳定键名。
    static constexpr auto Key = "database";
    /// 配置目录下的持久化文件名。
    static constexpr auto FileName = "database.json";

    /// 当前数据库驱动；当前版本只接受 @c sqlite。
    std::string driver = "sqlite";
    /// SQLite 驱动设置。
    SqliteConfig sqlite;
    /// 为后续 MySQL 驱动保留的连接设置。
    MySqlConfig mysql;

    /**
     * @brief 规范化数据库驱动名和 SQLite 同步模式，并解析数据库文件的绝对路径。
     * @param[in,out] config 待规范化的数据库配置。
     */
    static void normalize(DatabaseConfig &config);

    /**
     * @brief 校验当前应用支持的数据库配置。
     * @param[in] config 待校验的数据库配置。
     * @return 配置问题列表；空列表表示配置有效。
     */
    [[nodiscard]] static QList<Issue> validate(const DatabaseConfig &config);
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SqliteConfig, file_path, pool_size, busy_timeout_ms, wal_enabled, foreign_keys_enabled, synchronous)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(MySqlConfig, host, port, database, username, password, connect_timeout_seconds, pool_size, charset)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(DatabaseConfig, driver, sqlite, mysql)

} // namespace Config

#endif // DATABASECONFIG_H
