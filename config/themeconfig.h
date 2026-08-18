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
#include <vector>

namespace Config
{

/**
 * @brief 保存一个可选择的窗口背景。
 */
struct BackgroundImageConfig final
{
    /// 背景的显示名称。
    std::string name;
    /// 内置 qrc URL 或本地 file URL。
    std::string source;
    /// 是否在背景选择列表中启用该项。
    bool enabled = true;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(BackgroundImageConfig, name, source, enabled)

/**
 * @brief 保存界面主题、强调色、背景与动画设置。
 */
struct ThemeConfig final : ConfigBase<ThemeConfig>
{
    /// 配置注册表、变更信号使用的稳定键名。
    static constexpr auto Key = "theme";
    /// 配置目录下的持久化文件名。
    static constexpr auto FileName = "theme.json";

    /**
     * @brief 使用内置背景目录构造默认主题配置。
     */
    ThemeConfig();

    /// 主题模式，可取 @c light、@c dark 或 @c system。
    std::string mode = "dark";
    /// 界面强调色，使用 @c #RRGGBB 文本表示。
    std::string primary_color = "#4F7CFF";
    /// 是否启用界面过渡动画。
    bool animations_enabled = true;
    /// 窗口背景图片 URL；支持内置 qrc URL 或本地 file URL。
    std::string background_image = "qrc:/yuelink/assets/backgrounds/mist-blue-sakura-pink.png";
    /// 覆盖背景图片的主题表面不透明度，取值范围为 0.0 到 1.0。
    double background_opacity = 0.6;
    /// 可供选择的背景图片列表，包含内置默认项和用户导入项。
    std::vector<BackgroundImageConfig> background_images;

    /**
     * @brief 规范化主题模式、主题色和背景字段。
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

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ThemeConfig, mode, primary_color, animations_enabled, background_image, background_opacity, background_images)

} // namespace Config

#endif // THEMECONFIG_H
