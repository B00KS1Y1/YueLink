/**
 * @file identityconfig.h
 * @brief 定义本机设备身份配置及其 JSON 自动映射。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-08
 */

#ifndef IDENTITYCONFIG_H
#define IDENTITYCONFIG_H

#include "configbase.h"

#include <nlohmann/json.hpp>

#include <string>

namespace Config
{

/**
 * @brief 保存当前设备在局域网通信中展示的身份信息。
 *
 * 该类型没有额外规范化或校验策略，调用方负责生成设备标识并保证展示字段可用。
 */
struct IdentityConfig final : ConfigBase<IdentityConfig>
{
    /// 配置注册表、变更信号使用的稳定键名。
    static constexpr auto Key = "identity";
    /// 配置目录下的持久化文件名。
    static constexpr auto FileName = "identity.json";

    /// 当前设备的持久化唯一标识；首次使用时可为空，由身份初始化流程补全。
    std::string device_id;
    /// 向其他设备展示的用户名称。
    std::string display_name;
    /// 头像文件路径；空值表示没有自定义头像。
    std::string avatar_path;
    /// 头像占位背景色，使用 @c #RRGGBB 文本表示。
    std::string avatar_color = "#4F7CFF";
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(IdentityConfig, device_id, display_name, avatar_path, avatar_color)

} // namespace Config

#endif // IDENTITYCONFIG_H
