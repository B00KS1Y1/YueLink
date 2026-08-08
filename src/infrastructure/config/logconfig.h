/**
 * @file logconfig.h
 * @brief 定义日志配置及其 JSON 自动映射。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef LOGCONFIG_H
#define LOGCONFIG_H

#include "configbase.h"

#include <nlohmann/json.hpp>

#include <cstddef>
#include <string>

namespace Config
{

struct LogConfig final : ConfigBase<LogConfig>
{
    static constexpr auto Key = "log";
    static constexpr auto FileName = "log.json";

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

    /**
     * @brief 规范化日志级别和输出格式，并解析日志文件的绝对路径。
     * @param[in,out] config 待规范化的日志配置。
     */
    static void normalize(LogConfig &config);
    /**
     * @brief 校验日志配置。
     * @param[in] config 待校验的日志配置。
     * @return 配置问题列表。
     */
    [[nodiscard]] static QList<Issue> validate(const LogConfig &config);
};

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

} // namespace Config

#endif // LOGCONFIG_H
