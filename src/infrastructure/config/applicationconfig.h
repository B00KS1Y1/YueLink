/**
 * @file applicationconfig.h
 * @brief 定义应用程序配置及其 JSON 自动映射。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef APPLICATIONCONFIG_H
#define APPLICATIONCONFIG_H

#include "configbase.h"

#include <nlohmann/json.hpp>

#include <string>

namespace Config
{

struct ApplicationConfig final : ConfigBase<ApplicationConfig>
{
    static constexpr auto Key = "application";
    static constexpr auto FileName = "application.json";

    bool notifications_enabled = true;
    std::string download_directory;
    std::string config_directory;

    /**
     * @brief 创建包含默认下载目录的应用程序配置。
     * @param[in] context 配置运行时路径上下文。
     * @return 应用程序默认配置。
     */
    [[nodiscard]] static ApplicationConfig defaults(const ConfigContext &context);
    /**
     * @brief 规范化下载目录路径。
     * @param[in,out] config 待规范化的应用程序配置。
     * @param[in] context 配置运行时路径上下文。
     */
    static void normalize(ApplicationConfig &config, const ConfigContext &context);
    /**
     * @brief 校验应用程序配置。
     * @param[in] config 待校验的应用程序配置。
     * @param[in] context 配置运行时路径上下文。
     * @return 配置问题列表。
     */
    [[nodiscard]] static QList<Issue> validate(const ApplicationConfig &config, const ConfigContext &context);
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ApplicationConfig, notifications_enabled, download_directory, config_directory)

} // namespace Config

#endif // APPLICATIONCONFIG_H
