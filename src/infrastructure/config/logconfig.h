/**
 * @file logconfig.h
 * @brief 定义日志配置及其 JSON 自动映射。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-08
 */

#ifndef LOGCONFIG_H
#define LOGCONFIG_H

#include "configbase.h"

#include <nlohmann/json.hpp>

#include <cstddef>
#include <string>

namespace Config
{

/**
 * @brief 保存 spdlog 的过滤、输出、滚动文件和异步队列设置。
 */
struct LogConfig final : ConfigBase<LogConfig>
{
    /// 配置注册表、变更信号使用的稳定键名。
    static constexpr auto Key = "log";
    /// 配置目录下的持久化文件名。
    static constexpr auto FileName = "log.json";

    /// 最低输出级别，可取 trace、debug、info、warn、error、critical 或 off。
    std::string level = "info";
    /// spdlog 格式化模式；空值在规范化阶段恢复为默认模式。
    std::string pattern = "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v";
    /// 触发即时刷新的最低日志级别，取值集合与 @ref LogConfig::level 相同。
    std::string flush_level = "warn";
    /// 周期刷新间隔，单位为秒；0 表示不启用周期刷新。
    std::size_t flush_every_seconds = 0;

    /// 是否启用控制台输出。
    bool console_enabled = true;
    /// 是否为控制台级别文本着色；仅在启用控制台输出时生效。
    bool console_color = true;

    /// 是否启用滚动文件输出。
    bool file_enabled = true;
    /// 日志文件路径；相对路径按应用日志目录解析。
    std::string file_path = "yuelink.log";
    /// 单个滚动日志文件的最大字节数，启用文件输出时必须大于零。
    std::size_t max_file_size = 5 * 1024 * 1024;
    /// 保留的滚动日志文件数量，启用文件输出时必须大于零。
    std::size_t max_files = 3;
    /// 是否在日志系统启动时立即轮转已有文件。
    bool rotate_on_open = false;

    /// 是否使用 spdlog 异步日志器。
    bool async = false;
    /// 异步日志队列容量，启用异步模式时必须大于零；队列满时日志调用方会阻塞等待。
    std::size_t async_queue_size = 8192;
    /// 异步日志工作线程数，启用异步模式时必须大于零。
    std::size_t async_thread_count = 1;

    /**
     * @brief 规范化日志级别和输出格式，并解析日志文件的绝对路径。
     * @param[in,out] config 待规范化的日志配置。
     */
    static void normalize(LogConfig &config);

    /**
     * @brief 校验日志配置。
     * @param[in] config 待校验的日志配置。
     * @return 配置问题列表；空列表表示配置有效。
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
