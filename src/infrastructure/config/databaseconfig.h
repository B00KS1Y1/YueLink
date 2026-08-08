/**
 * @file databaseconfig.h
 * @brief 定义数据库配置及其 JSON 自动映射。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
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

struct SqliteConfig
{
    std::string file_path = "yuelink.db";
    std::size_t pool_size = 1;
    std::size_t busy_timeout_ms = 5000;
    bool wal_enabled = true;
    bool foreign_keys_enabled = true;
    std::string synchronous = "normal";
};

struct MySqlConfig
{
    std::string host = "127.0.0.1";
    std::uint16_t port = 3306;
    std::string database = "yuelink";
    std::string username = "root";
    std::string password;
    std::size_t connect_timeout_seconds = 5;
    std::size_t pool_size = 4;
    std::string charset = "utf8mb4";
};

struct DatabaseConfig final : ConfigBase<DatabaseConfig>
{
    static constexpr auto Key = "database";
    static constexpr auto FileName = "database.json";

    std::string driver = "sqlite";
    SqliteConfig sqlite;
    MySqlConfig mysql;

    /**
     * @brief 规范化数据库驱动名、文件路径和 SQLite 同步模式。
     * @param[in,out] config 待规范化的数据库配置。
     * @param[in] context 配置运行时路径上下文。
     */
    static void normalize(DatabaseConfig &config, const ConfigContext &context);
    /**
     * @brief 校验当前应用支持的数据库配置。
     * @param[in] config 待校验的数据库配置。
     * @param[in] context 配置运行时路径上下文。
     * @return 配置问题列表。
     */
    [[nodiscard]] static QList<Issue> validate(const DatabaseConfig &config, const ConfigContext &context);
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SqliteConfig, file_path, pool_size, busy_timeout_ms, wal_enabled, foreign_keys_enabled, synchronous)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(MySqlConfig, host, port, database, username, password, connect_timeout_seconds, pool_size, charset)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(DatabaseConfig, driver, sqlite, mysql)

} // namespace Config

#endif // DATABASECONFIG_H
