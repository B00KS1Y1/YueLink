/**
 * @file configmanager.cpp
 * @brief 实现内置配置存储的批量初始化、重新加载和结果聚合。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-08
 */

#include "configmanager.h"

#include <type_traits>

namespace
{
/**
 * @brief 将单个配置操作的状态合并到批量操作结果。
 * @param[in,out] aggregate 已处理配置的聚合结果。
 * @param[in] current 当前配置操作结果。
 *
 * 聚合结果保留首个错误的分类和文件路径、收集全部校验问题，并拼接不同的错误摘要。
 */
void mergeResult(Config::Result &aggregate, const Config::Result &current)
{
    aggregate.changed = aggregate.changed || current.changed;
    aggregate.issues.append(current.issues);
    if (current)
    {
        return;
    }
    if (aggregate)
    {
        // 首个错误提供批量结果的主错误码和定位路径，后续错误仅追加摘要。
        aggregate.errorCode = current.errorCode;
        aggregate.filePath = current.filePath;
        aggregate.errorMessage = current.errorMessage;
        return;
    }
    if (!aggregate.errorMessage.contains(current.errorMessage))
    {
        aggregate.errorMessage += QStringLiteral("；%1").arg(current.errorMessage);
    }
}
} // namespace

namespace Config
{

ConfigManager::ConfigManager()
: QObject(nullptr)
{
}

ConfigManager &ConfigManager::instance()
{
    static ConfigManager manager;
    return manager;
}

Result ConfigManager::initialize(const QString &configDirectory)
{
    Result aggregate;
    // 存储元组由注册表在编译期生成；折叠表达式保证新增注册类型自动参与批量初始化。
    std::apply(
        [&aggregate, &configDirectory](auto &...stores) {
            const auto initializeOne = [&aggregate, &configDirectory](auto &store) {
                const Result result = store.initialize(configDirectory);
                mergeResult(aggregate, result);
            };
            (initializeOne(stores), ...);
        },
        m_stores);
    return aggregate;
}

Result ConfigManager::reloadAll()
{
    Result aggregate;
    // 每个存储独立加载并通知，单个失败不会中断后续配置。
    std::apply(
        [this, &aggregate](auto &...stores) {
            const auto reloadOne = [this, &aggregate](auto &store) {
                using Store = std::decay_t<decltype(store)>;
                using Value = typename Store::ValueType;
                const Result result = store.reload();
                mergeResult(aggregate, result);
                notifyChanged<Value>(result);
            };
            (reloadOne(stores), ...);
        },
        m_stores);
    return aggregate;
}

} // namespace Config
