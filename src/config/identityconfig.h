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
 * 首次使用时允许设备标识和展示名称为空，由身份初始化流程补全；非空持久化值必须满足
 * UUID、长度、路径和颜色格式约束。
 */
struct IdentityConfig final : ConfigBase<IdentityConfig>
{
    /// 配置注册表、变更信号使用的稳定键名。
    static constexpr auto Key = "identity";
    /// 配置目录下的持久化文件名。
    static constexpr auto FileName = "identity.json";

    /// 当前设备的持久化 UUID；首次使用时可为空，由身份初始化流程补全。
    std::string device_id;
    /// 向其他设备展示的用户名称；首次使用时可为空，非空时最多包含 64 个字符。
    std::string display_name;
    /// 头像文件绝对路径；空值表示没有自定义头像，持久化长度上限为 4096 个字符。
    std::string avatar_path;
    /// 头像占位背景色，使用 @c #RRGGBB 文本表示。
    std::string avatar_color = "#4F7CFF";

    /**
     * @brief 规范化设备标识、展示名称、头像路径和头像颜色。
     * @param[in,out] config 待规范化的本机身份配置。
     */
    static void normalize(IdentityConfig &config);

    /**
     * @brief 校验本机身份配置的持久化边界。
     * @param[in] config 待校验的本机身份配置。
     * @return 配置问题列表；空列表表示配置有效。
     */
    [[nodiscard]] static QList<Issue> validate(const IdentityConfig &config);
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(IdentityConfig, device_id, display_name, avatar_path, avatar_color)

} // namespace Config

#endif // IDENTITYCONFIG_H
