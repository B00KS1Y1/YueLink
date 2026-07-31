/**
 * @file configserializer.h
 * @brief 定义配置结构与 JSON 之间的序列化映射。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef CONFIGSERIALIZER_H
#define CONFIGSERIALIZER_H

#include "config.h"

#include <nlohmann/json.hpp>

namespace Config
{

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(IdentityConfig,
                                                device_id,
                                                display_name)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ApplicationConfig,
                                                notifications_enabled)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ThemeConfig,
                                                mode,
                                                primary_color,
                                                animations_enabled,
                                                navigation_mode,
                                                dark_background,
                                                light_background)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(LogConfig,
                                                level,
                                                pattern,
                                                flush_level,
                                                flush_every_seconds,
                                                console_enabled,
                                                console_color,
                                                file_enabled,
                                                file_path,
                                                max_file_size,
                                                max_files,
                                                rotate_on_open,
                                                async,
                                                async_queue_size,
                                                async_thread_count)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SqliteConfig,
                                                file_path,
                                                pool_size,
                                                busy_timeout_ms,
                                                wal_enabled,
                                                foreign_keys_enabled,
                                                synchronous)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(MySqlConfig,
                                                host,
                                                port,
                                                database,
                                                username,
                                                password,
                                                connect_timeout_seconds,
                                                pool_size,
                                                charset)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(DatabaseConfig,
                                                driver,
                                                sqlite,
                                                mysql)

} // namespace Config

#endif // CONFIGSERIALIZER_H
