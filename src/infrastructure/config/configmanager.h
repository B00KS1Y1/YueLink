/**
 * @file configmanager.h
 * @brief 声明所有已注册配置的统一管理器。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-06
 */

#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include "configregistry.h"
#include "configstore.h"

#include <QObject>
#include <QString>

#include <tuple>
#include <type_traits>
#include <utility>

namespace Config
{

template <typename List> struct StoreTuple;

template <typename... Types> struct StoreTuple<ConfigList<Types...>>
{
    using Type = std::tuple<ConfigStore<Types>...>;
};

class ConfigManager final : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 返回进程内唯一配置管理器。
     * @return 配置管理器引用；对象生命周期持续到进程退出。
     */
    [[nodiscard]] static ConfigManager &instance();

    /**
     * @brief 初始化并加载全部已注册配置。
     * @return 聚合加载结果；单个配置失败不阻止其他配置加载。
     */
    [[nodiscard]] Result initialize();
    /**
     * @brief 从各自文件重新加载全部已注册配置。
     * @return 聚合重新加载结果。
     */
    [[nodiscard]] Result reloadAll();

    /**
     * @brief 返回指定类型的当前配置副本。
     * @tparam T 已注册配置类型。
     * @return 当前配置。
     */
    template <typename T> [[nodiscard]] T get() const
    {
        return store<T>().get();
    }

    /**
     * @brief 返回指定类型的带修订号配置快照。
     * @tparam T 已注册配置类型。
     * @return 当前配置和修订号。
     */
    template <typename T> [[nodiscard]] Snapshot<T> snapshot() const
    {
        return store<T>().snapshot();
    }

    /**
     * @brief 校验并保存指定类型的完整配置。
     * @tparam T 已注册配置类型。
     * @param[in] candidate 候选配置。
     * @return 保存结果。
     */
    template <typename T> [[nodiscard]] Result set(T candidate)
    {
        Result result = store<T>().set(std::move(candidate));
        notifyChanged<T>(result);
        return result;
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
        Result result = store<T>().update(std::forward<Mutator>(mutator));
        notifyChanged<T>(result);
        return result;
    }

    /**
     * @brief 从文件重新加载指定类型配置。
     * @tparam T 已注册配置类型。
     * @return 重新加载结果。
     */
    template <typename T> [[nodiscard]] Result reload()
    {
        Result result = store<T>().reload();
        notifyChanged<T>(result);
        return result;
    }

    /**
     * @brief 恢复并保存指定类型的默认配置。
     * @tparam T 已注册配置类型。
     * @return 重置结果。
     */
    template <typename T> [[nodiscard]] Result reset()
    {
        Result result = store<T>().reset();
        notifyChanged<T>(result);
        return result;
    }

    /**
     * @brief 返回指定类型配置文件的绝对路径。
     * @tparam T 已注册配置类型。
     * @return 配置文件路径。
     */
    template <typename T> [[nodiscard]] QString filePath() const
    {
        return store<T>().filePath();
    }

signals:
    /**
     * @brief 成功发布新的配置值后发出。
     * @param[in] configKey 配置类型的稳定键名。
     * @param[in] revision 新的配置修订号。
     */
    void configChanged(const QString &configKey, quint64 revision);

private:
    /** @brief 构造配置管理器。 */
    ConfigManager();

    /**
     * @brief 返回指定类型的可写配置存储。
     * @tparam T 已注册配置类型。
     * @return 类型化配置存储引用。
     */
    template <typename T> [[nodiscard]] ConfigStore<T> &store()
    {
        static_assert(IsRegisteredV<T>, "The requested configuration type is not registered.");
        return std::get<ConfigStore<T>>(m_stores);
    }

    /**
     * @brief 返回指定类型的只读配置存储。
     * @tparam T 已注册配置类型。
     * @return 类型化配置存储常量引用。
     */
    template <typename T> [[nodiscard]] const ConfigStore<T> &store() const
    {
        static_assert(IsRegisteredV<T>, "The requested configuration type is not registered.");
        return std::get<ConfigStore<T>>(m_stores);
    }

    /**
     * @brief 根据操作结果发送指定配置的变更信号。
     * @tparam T 已注册配置类型。
     * @param[in] result 配置操作结果。
     */
    template <typename T> void notifyChanged(const Result &result)
    {
        if (result && result.changed)
        {
            emit configChanged(QString::fromLatin1(T::Key), result.revision);
        }
    }

    using Stores = typename StoreTuple<BuiltInConfigs>::Type;
    Stores m_stores;
};

} // namespace Config

#endif // CONFIGMANAGER_H
