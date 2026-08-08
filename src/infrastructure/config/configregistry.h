/**
 * @file configregistry.h
 * @brief 注册应用程序内置配置类型。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-06
 */

#ifndef CONFIGREGISTRY_H
#define CONFIGREGISTRY_H

#include "config.h"

#include <type_traits>

namespace Config
{

template <typename... Types> struct ConfigList
{
};

using BuiltInConfigs = ConfigList<IdentityConfig, ApplicationConfig, ThemeConfig, LogConfig, DatabaseConfig>;

template <typename T, typename List> struct IsRegistered;

template <typename T, typename... Types> struct IsRegistered<T, ConfigList<Types...>> : std::disjunction<std::is_same<T, Types>...>
{
};

template <typename T> inline constexpr bool IsRegisteredV = IsRegistered<T, BuiltInConfigs>::value;

template <typename List> struct AreConfigTypes;

template <typename... Types> struct AreConfigTypes<ConfigList<Types...>> : std::conjunction<std::is_base_of<ConfigBase<Types>, Types>...>
{
};

static_assert(AreConfigTypes<BuiltInConfigs>::value, "All registered configuration types must derive from ConfigBase<T>.");

} // namespace Config

#endif // CONFIGREGISTRY_H
