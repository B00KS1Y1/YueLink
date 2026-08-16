/**
 * @file runtimebootstrap.h
 * @brief 声明共享运行时的初始化与关闭函数。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-24
 */

#ifndef RUNTIMEBOOTSTRAP_H
#define RUNTIMEBOOTSTRAP_H

namespace RuntimeBootstrap
{

/** @brief 加载共享配置并初始化日志系统。 */
void initialize();
/** @brief 按依赖顺序关闭共享运行时服务。 */
void shutdown();

} // namespace RuntimeBootstrap

#endif // RUNTIMEBOOTSTRAP_H
