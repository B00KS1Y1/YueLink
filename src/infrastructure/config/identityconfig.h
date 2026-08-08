/**
 * @file identityconfig.h
 * @brief 定义本机设备身份配置及其 JSON 自动映射。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef IDENTITYCONFIG_H
#define IDENTITYCONFIG_H

#include "configbase.h"

#include <nlohmann/json.hpp>

#include <string>

namespace Config
{

struct IdentityConfig final : ConfigBase<IdentityConfig>
{
    static constexpr auto Key = "identity";
    static constexpr auto FileName = "identity.json";

    std::string device_id;
    std::string display_name;
    std::string avatar_path;
    std::string avatar_color = "#4F7CFF";
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(IdentityConfig, device_id, display_name, avatar_path, avatar_color)

} // namespace Config

#endif // IDENTITYCONFIG_H
