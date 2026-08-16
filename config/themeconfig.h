/**
 * @file themeconfig.h
 * @brief 定义主题配置及其 JSON 自动映射。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-08
 */

#ifndef THEMECONFIG_H
#define THEMECONFIG_H

#include "configbase.h"

#include <nlohmann/json.hpp>

#include <string>

namespace Config
{

/**
 * @brief 保存界面主题、强调色与动画设置。
 */
struct ThemeConfig final : ConfigBase<ThemeConfig>
{
    /// 配置注册表、变更信号使用的稳定键名。
    static constexpr auto Key = "theme";
    /// 配置目录下的持久化文件名。
    static constexpr auto FileName = "theme.json";

    /// 主题模式，可取 @c light、@c dark 或 @c system。
    std::string mode = "dark";
    /// 界面强调色，使用 @c #RRGGBB 文本表示。
    std::string primary_color = "#4F7CFF";
    /// 是否启用界面过渡动画。
    bool animations_enabled = true;
    /// 可选的深色背景色；空值表示不指定自定义颜色。
    std::string dark_background;
    /// 可选的浅色背景色；空值表示不指定自定义颜色。
    std::string light_background;

    /**
     * @brief 规范化主题模式和颜色值。
     * @param[in,out] config 待规范化的主题配置。
     */
    static void normalize(ThemeConfig &config);

    /**
     * @brief 校验主题配置。
     * @param[in] config 待校验的主题配置。
     * @return 配置问题列表；空列表表示配置有效。
     */
    [[nodiscard]] static QList<Issue> validate(const ThemeConfig &config);
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ThemeConfig, mode, primary_color, animations_enabled, dark_background, light_background)

} // namespace Config

#endif // THEMECONFIG_H
