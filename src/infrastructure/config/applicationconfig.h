/**
 * @file applicationconfig.h
 * @brief 定义应用程序配置及其 JSON 自动映射。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef APPLICATIONCONFIG_H
#define APPLICATIONCONFIG_H

#include "configbase.h"

#include "infrastructure/path.h"

#include <nlohmann/json.hpp>

#include <string>

namespace Config
{

struct ApplicationConfig final : ConfigBase<ApplicationConfig>
{
    static constexpr auto Key = "application";
    static constexpr auto FileName = "application.json";

    bool notifications_enabled = true;
    std::string download_directory = Path::DownloadDirectory().toStdString();

    /**
     * @brief 规范化下载目录路径。
     * @param[in,out] config 待规范化的应用程序配置。
     */
    static void normalize(ApplicationConfig &config);
    /**
     * @brief 校验应用程序配置。
     * @param[in] config 待校验的应用程序配置。
     * @return 配置问题列表。
     */
    [[nodiscard]] static QList<Issue> validate(const ApplicationConfig &config);
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ApplicationConfig, notifications_enabled, download_directory)

} // namespace Config

#endif // APPLICATIONCONFIG_H
