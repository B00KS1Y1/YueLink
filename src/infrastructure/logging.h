/**
 * @file logging.h
 * @brief 声明进程级日志系统的生命周期函数。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef LOGGING_H
#define LOGGING_H

#include "config/config.h"
#include "config/result.h"

namespace Logging
{

/**
 * @brief 根据配置初始化进程级日志系统。
 * @param config 日志输出、级别、轮转与异步选项。
 * @return 初始化操作结果。
 */
[[nodiscard]] Config::Result initialize(const Config::LogConfig &config);

/** @brief 刷新并关闭进程级日志系统。 */
void shutdown();

} // namespace Logging

#endif // LOGGING_H
