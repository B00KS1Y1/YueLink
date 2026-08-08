/**
 * @file configapi.h
 * @brief 提供与具体存储实现解耦的统一配置访问接口。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-06
 */

#ifndef CONFIGAPI_H
#define CONFIGAPI_H

#include "configmanager.h"

#include <QString>

#include <utility>

namespace Config
{

/**
 * @brief 初始化并加载全部已注册配置。
 * @return 聚合加载结果。
 */
[[nodiscard]] inline Result initialize()
{
    return ConfigManager::instance().initialize();
}

/**
 * @brief 重新加载全部已注册配置。
 * @return 聚合重新加载结果。
 */
[[nodiscard]] inline Result reloadAll()
{
    return ConfigManager::instance().reloadAll();
}

/**
 * @brief 返回指定类型的当前配置副本。
 * @tparam T 已注册配置类型。
 * @return 当前配置。
 */
template <typename T> [[nodiscard]] T get()
{
    return ConfigManager::instance().get<T>();
}

/**
 * @brief 返回指定类型的带修订号配置快照。
 * @tparam T 已注册配置类型。
 * @return 当前配置和修订号。
 */
template <typename T> [[nodiscard]] Snapshot<T> snapshot()
{
    return ConfigManager::instance().snapshot<T>();
}

/**
 * @brief 校验并保存指定类型的完整配置。
 * @tparam T 已注册配置类型。
 * @param[in] candidate 候选配置。
 * @return 保存结果。
 */
template <typename T> [[nodiscard]] Result set(T candidate)
{
    return ConfigManager::instance().set<T>(std::move(candidate));
}

/**
 * @brief 修改并保存指定类型配置。
 * @tparam T 已注册配置类型。
 * @tparam Mutator 可调用修改器类型。
 * @param[in] mutator 接收 @c T& 的配置修改器。
 * @return 更新结果。
 */
template <typename T, typename Mutator> [[nodiscard]] Result update(Mutator &&mutator)
{
    return ConfigManager::instance().update<T>(std::forward<Mutator>(mutator));
}

/**
 * @brief 重新加载指定类型配置。
 * @tparam T 已注册配置类型。
 * @return 重新加载结果。
 */
template <typename T> [[nodiscard]] Result reload()
{
    return ConfigManager::instance().reload<T>();
}

/**
 * @brief 恢复并保存指定类型的默认配置。
 * @tparam T 已注册配置类型。
 * @return 重置结果。
 */
template <typename T> [[nodiscard]] Result reset()
{
    return ConfigManager::instance().reset<T>();
}

/**
 * @brief 返回指定类型配置文件的绝对路径。
 * @tparam T 已注册配置类型。
 * @return 配置文件路径。
 */
template <typename T> [[nodiscard]] QString filePath()
{
    return ConfigManager::instance().filePath<T>();
}

} // namespace Config

#endif // CONFIGAPI_H
