/**
 * @file config.h
 * @brief 定义应用程序持久化配置数据结构。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <cstddef>
#include <cstdint>
#include <string>

namespace Config
{

struct ApplicationConfig
{
    bool notifications_enabled = true;
};

struct ThemeConfig
{
    std::string mode = "dark";
    std::string primary_color = "#4F7CFF";
    bool animations_enabled = true;
    std::string navigation_mode = "relaxed";
    std::string dark_background;
    std::string light_background;
};

struct LogConfig
{
    std::string level = "info";
    std::string pattern = "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v";
    std::string flush_level = "warn";
    std::size_t flush_every_seconds = 0;

    bool console_enabled = true;
    bool console_color = true;

    bool file_enabled = true;
    std::string file_path = "yuelink.log";
    std::size_t max_file_size = 5 * 1024 * 1024;
    std::size_t max_files = 3;
    bool rotate_on_open = false;

    bool async = false;
    std::size_t async_queue_size = 8192;
    std::size_t async_thread_count = 1;
};

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

struct DatabaseConfig
{
    std::string driver = "sqlite";
    SqliteConfig sqlite;
    MySqlConfig mysql;
};

} // namespace Config

#endif // CONFIG_H
