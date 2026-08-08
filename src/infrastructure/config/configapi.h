/**
 * @file configapi.h
 * @brief 提供与具体存储实现解耦的统一配置访问接口。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-08
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
 * @param[in] configDirectory 配置文件所在目录。
 * @return 聚合加载结果；单个配置失败不阻止其他配置加载。
 */
[[nodiscard]] inline Result initialize(const QString &configDirectory)
{
    return ConfigManager::instance().initialize(configDirectory);
}

/**
 * @brief 重新加载全部已注册配置。
 * @return 聚合重新加载结果；成功发布新值的配置会发送变更信号。
 */
[[nodiscard]] inline Result reloadAll()
{
    return ConfigManager::instance().reloadAll();
}

/**
 * @brief 返回指定类型的当前配置副本。
 * @tparam T 已注册配置类型。
 * @return 受读锁保护的一致配置副本。
 */
template <typename T> [[nodiscard]] T get()
{
    return ConfigManager::instance().get<T>();
}

/**
 * @brief 返回指定类型的带修订号配置快照。
 * @tparam T 已注册配置类型。
 * @return 在同一次读锁中取得的当前配置和修订号。
 */
template <typename T> [[nodiscard]] Snapshot<T> snapshot()
{
    return ConfigManager::instance().snapshot<T>();
}

/**
 * @brief 规范化、校验并原子保存指定类型的完整配置。
 * @tparam T 已注册配置类型。
 * @param[in] candidate 候选配置。
 * @return 保存结果；保存失败时原文件和当前内存值均保持不变。
 * @note 即使候选值未变化也会确保配置文件已写入。
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
 * @return 更新结果；并发写入导致快照过期时返回 @c ErrorCode::Conflict。
 * @note 修改器在存储锁之外执行，抛出的异常会转换为 @c ErrorCode::MutationFailed。
 */
template <typename T, typename Mutator> [[nodiscard]] Result update(Mutator &&mutator)
{
    return ConfigManager::instance().update<T>(std::forward<Mutator>(mutator));
}

/**
 * @brief 重新加载指定类型配置。
 * @tparam T 已注册配置类型。
 * @return 重新加载结果；失败时保留当前内存值。
 */
template <typename T> [[nodiscard]] Result reload()
{
    return ConfigManager::instance().reload<T>();
}

/**
 * @brief 恢复并保存指定类型的默认配置。
 * @tparam T 已注册配置类型。
 * @return 重置结果；保存失败时保留当前内存值。
 */
template <typename T> [[nodiscard]] Result reset()
{
    return ConfigManager::instance().reset<T>();
}

/**
 * @brief 返回指定类型配置文件的绝对路径。
 * @tparam T 已注册配置类型。
 * @return 配置文件路径；对应存储初始化前返回空字符串。
 */
template <typename T> [[nodiscard]] QString filePath()
{
    return ConfigManager::instance().filePath<T>();
}

} // namespace Config

#endif // CONFIGAPI_H
