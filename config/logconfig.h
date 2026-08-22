/**
 * @file logconfig.h
 * @brief 定义 QyLog 配置及其 JSON 自动映射。
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
 * @brief 保存 QyLog 的过滤、源码位置、写入线程、控制台输出与滚动文件设置。
 */
struct LogConfig final : ConfigBase<LogConfig>
{
    /// 配置注册表和变更信号使用的稳定键名。
    static constexpr auto Key = "log";
    /// 配置目录下的持久化文件名。
    static constexpr auto FileName = "log.json";

    /// 最低输出级别，可取 trace、debug、info、warn、error、critical 或 off。
    std::string level = "info";
    /// 是否在每条日志中包含调用点的源码文件路径和行号。
    bool source_location_enabled = false;
    /// 是否通过 QyLog 的专用单线程队列异步写入日志。
    bool separate_thread_enabled = true;
    /// 是否启用调试控制台输出。
    bool console_enabled = true;
    /// 是否启用滚动文件输出。
    bool file_enabled = true;
    /// 日志文件路径；相对路径按应用日志目录解析。
    std::string file_path = "yuelink.log";
    /// 单个滚动日志文件的最大字节数，启用文件输出时必须大于零。
    std::size_t max_file_size = 5 * 1024 * 1024;
    /// 保留的旧日志文件数量，启用文件输出时必须大于零；QyLog 最多保留 10 个。
    std::size_t max_files = 3;

    /**
     * @brief 规范化日志级别并解析日志文件的绝对路径。
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

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
    LogConfig, level, source_location_enabled, separate_thread_enabled, console_enabled, file_enabled, file_path, max_file_size, max_files)

} // namespace Config

#endif // LOGCONFIG_H
