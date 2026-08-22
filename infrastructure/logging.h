/**
 * @file logging.h
 * @brief 声明基于 QyLog 的进程级日志系统生命周期函数。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef LOGGING_H
#define LOGGING_H

#include "config/config.h"
#include "config/result.h"

#include <string>

namespace Logging
{

/**
 * @brief 根据配置初始化进程级 QyLog 日志系统。
 * @param[in] config 日志级别、源码位置、写入线程、控制台输出和滚动文件设置。
 * @return 初始化成功时返回成功结果，否则返回包含错误原因的失败结果并启用控制台回退日志。
 */
[[nodiscard]] Config::Result initialize(const Config::LogConfig &config);

/**
 * @brief 更新当前 QyLog 的最低输出级别。
 * @param[in] level 日志级别名称；无效值回退为 info。
 */
void setLevel(const std::string &level);

/** @brief 等待待写入消息完成并关闭进程级 QyLog 日志系统。 */
void shutdown();

} // namespace Logging

#endif // LOGGING_H
