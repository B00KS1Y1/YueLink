/**
 * @file configmanager.h
 * @brief 声明所有已注册配置的统一管理器。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-08
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

/**
 * @brief 将配置类型清单转换为对应的存储元组类型。
 * @tparam List 使用 @c ConfigList 表示的配置类型清单。
 */
template <typename List> struct StoreTuple;

/**
 * @brief 为清单中的每个配置类型生成一个 @c ConfigStore。
 * @tparam Types 已注册配置类型。
 */
template <typename... Types> struct StoreTuple<ConfigList<Types...>>
{
    /// 与配置类型清单顺序一致的类型化存储元组。
    using Type = std::tuple<ConfigStore<Types>...>;
};

/**
 * @brief 统一管理全部内置配置的加载、访问、持久化与变更通知。
 *
 * 每个配置类型拥有独立的 @c ConfigStore 和读写锁，因此不同线程可以安全读取或更新配置。
 * 变更信号在发起操作的线程中发射，跨线程接收方式由 Qt 连接类型决定。
 */
class ConfigManager final : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 返回进程内唯一配置管理器。
     * @return 配置管理器引用；对象生命周期持续到进程退出。
     * @note 单例的首次构造由 C++ 运行时保证线程安全。
     */
    [[nodiscard]] static ConfigManager &instance();

    /**
     * @brief 初始化并加载全部已注册配置。
     * @param[in] configDirectory 配置文件所在目录。
     * @return 聚合加载结果；包含首个错误及全部校验问题，单个配置失败不阻止其他配置加载。
     */
    [[nodiscard]] Result initialize(const QString &configDirectory);

    /**
     * @brief 从各自文件重新加载全部已注册配置。
     * @return 聚合重新加载结果；单个配置失败不阻止其他配置重新加载。
     */
    [[nodiscard]] Result reloadAll();

    /**
     * @brief 返回指定类型的当前配置副本。
     * @tparam T 已注册配置类型。
     * @return 受读锁保护的一致配置副本。
     */
    template <typename T> [[nodiscard]] T get() const
    {
        return store<T>().get();
    }

    /**
     * @brief 返回指定类型的带修订号配置快照。
     * @tparam T 已注册配置类型。
     * @return 在同一次读锁中取得的当前配置和修订号。
     */
    template <typename T> [[nodiscard]] Snapshot<T> snapshot() const
    {
        return store<T>().snapshot();
    }

    /**
     * @brief 规范化、校验并原子保存指定类型的完整配置。
     * @tparam T 已注册配置类型。
     * @param[in] candidate 候选配置。
     * @return 保存结果；即使配置值未变化也会确保文件已写入，但不会发送变更信号。
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
     * @return 更新结果；并发写入导致快照过期时返回 @c ErrorCode::Conflict。
     * @note 修改器在内部锁之外执行，不应依赖存储中的其他可变状态。
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
     * @return 重新加载结果；失败时保留当前内存值。
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
     * @return 重置结果；保存失败时保留当前内存值。
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
     * @return 配置文件路径；对应存储初始化前返回空字符串。
     */
    template <typename T> [[nodiscard]] QString filePath() const
    {
        return store<T>().filePath();
    }

signals:
    /**
     * @brief 通过设置、更新、重新加载或重置成功发布新的配置值后发出。
     * @param[in] configKey 配置类型的稳定键名。
     * @param[in] revision 新的配置修订号。
     * @note 初始化加载不发送该信号，值未变化的成功操作也不发送。
     */
    void configChanged(const QString &configKey, quint64 revision);

private:
    /**
     * @brief 构造不带 QObject 父对象的进程级配置管理器。
     */
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
     * @note 仅当操作成功且 @c result.changed 为 @c true 时发送信号。
     */
    template <typename T> void notifyChanged(const Result &result)
    {
        if (result && result.changed)
        {
            emit configChanged(QString::fromLatin1(T::Key), result.revision);
        }
    }

    using Stores = typename StoreTuple<BuiltInConfigs>::Type;
    Stores m_stores; ///< 每种内置配置各自独立的存储实例。
};

} // namespace Config

#endif // CONFIGMANAGER_H
