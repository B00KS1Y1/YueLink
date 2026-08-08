/**
 * @file themeconfig.h
 * @brief 定义主题配置及其 JSON 自动映射。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef THEMECONFIG_H
#define THEMECONFIG_H

#include "configbase.h"

#include <nlohmann/json.hpp>

#include <string>

namespace Config
{

struct ThemeConfig final : ConfigBase<ThemeConfig>
{
    static constexpr auto Key = "theme";
    static constexpr auto FileName = "theme.json";

    std::string mode = "dark";
    std::string primary_color = "#4F7CFF";
    bool animations_enabled = true;
    std::string navigation_mode = "compact";
    std::string dark_background;
    std::string light_background;

    /**
     * @brief 规范化主题模式、导航模式和颜色值。
     * @param[in,out] config 待规范化的主题配置。
     */
    static void normalize(ThemeConfig &config);
    /**
     * @brief 校验主题配置。
     * @param[in] config 待校验的主题配置。
     * @return 配置问题列表。
     */
    [[nodiscard]] static QList<Issue> validate(const ThemeConfig &config);
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ThemeConfig, mode, primary_color, animations_enabled, navigation_mode, dark_background, light_background)

} // namespace Config

#endif // THEMECONFIG_H
