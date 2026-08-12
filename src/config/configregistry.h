/**
 * @file configregistry.h
 * @brief 注册应用程序内置配置类型。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-08
 */

#ifndef CONFIGREGISTRY_H
#define CONFIGREGISTRY_H

#include "config.h"

#include <type_traits>

namespace Config
{

/**
 * @brief 在编译期保存一组配置类型。
 * @tparam Types 配置值类型列表。
 */
template <typename... Types> struct ConfigList
{
};

namespace Detail
{
/**
 * @brief 比较两个以空字符结尾的编译期文本是否相同。
 * @param[in] left 左侧文本。
 * @param[in] right 右侧文本。
 * @return 两个文本逐字符相同时返回 @c true。
 */
[[nodiscard]] constexpr bool equalText(const char *left, const char *right)
{
    while (*left != '\0' && *right != '\0')
    {
        if (*left != *right)
        {
            return false;
        }
        ++left;
        ++right;
    }
    return *left == *right;
}
} // namespace Detail

/**
 * @brief 应用内置配置的唯一注册清单。
 *
 * @c ConfigManager 根据该清单生成等长的类型化存储元组；新增配置必须加入此处。
 */
using BuiltInConfigs = ConfigList<IdentityConfig, ApplicationConfig, ThemeConfig, LogConfig, DatabaseConfig>;

/**
 * @brief 判断类型是否存在于指定配置类型清单中。
 * @tparam T 待判断的类型。
 * @tparam List 配置类型清单。
 */
template <typename T, typename List> struct IsRegistered;

template <typename T, typename... Types> struct IsRegistered<T, ConfigList<Types...>> : std::disjunction<std::is_same<T, Types>...>
{
};

/// 判断 @c T 是否已注册为内置配置类型。
template <typename T> inline constexpr bool IsRegisteredV = IsRegistered<T, BuiltInConfigs>::value;

/**
 * @brief 判断清单中的全部类型是否正确继承各自的 CRTP 配置基类。
 * @tparam List 配置类型清单。
 */
template <typename List> struct AreConfigTypes;

template <typename... Types> struct AreConfigTypes<ConfigList<Types...>> : std::conjunction<std::is_base_of<ConfigBase<Types>, Types>...>
{
};

/**
 * @brief 判断清单中全部配置类型的稳定键名是否唯一。
 * @tparam List 配置类型清单。
 */
template <typename List> struct AreConfigKeysUnique;

template <> struct AreConfigKeysUnique<ConfigList<>> : std::true_type
{
};

template <typename First, typename... Rest>
struct AreConfigKeysUnique<ConfigList<First, Rest...>>
    : std::bool_constant<((!Detail::equalText(First::Key, Rest::Key)) && ...) && AreConfigKeysUnique<ConfigList<Rest...>>::value>
{
};

/**
 * @brief 判断清单中全部配置类型的持久化文件名是否唯一。
 * @tparam List 配置类型清单。
 */
template <typename List> struct AreConfigFileNamesUnique;

template <> struct AreConfigFileNamesUnique<ConfigList<>> : std::true_type
{
};

template <typename First, typename... Rest>
struct AreConfigFileNamesUnique<ConfigList<First, Rest...>>
    : std::bool_constant<((!Detail::equalText(First::FileName, Rest::FileName)) && ...) &&
                         AreConfigFileNamesUnique<ConfigList<Rest...>>::value>
{
};

static_assert(AreConfigTypes<BuiltInConfigs>::value, "All registered configuration types must derive from ConfigBase<T>.");
static_assert(AreConfigKeysUnique<BuiltInConfigs>::value, "Registered configuration keys must be unique.");
static_assert(AreConfigFileNamesUnique<BuiltInConfigs>::value, "Registered configuration file names must be unique.");

} // namespace Config

#endif // CONFIGREGISTRY_H
