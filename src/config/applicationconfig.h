/**
 * @file applicationconfig.h
 * @brief 定义应用程序配置及其 JSON 自动映射。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-08
 */

#ifndef APPLICATIONCONFIG_H
#define APPLICATIONCONFIG_H

#include "configbase.h"

#include "infrastructure/path.h"

#include <nlohmann/json.hpp>

#include <string>

namespace Config
{

/**
 * @brief 保存通知与下载目录等应用级设置。
 *
 * 字段通过 nlohmann/json 直接映射到 @c application.json；缺失字段使用成员默认值。
 */
struct ApplicationConfig final : ConfigBase<ApplicationConfig>
{
    /// 配置注册表、变更信号使用的稳定键名。
    static constexpr auto Key = "application";
    /// 配置目录下的持久化文件名。
    static constexpr auto FileName = "application.json";

    /// 是否允许应用发送通知。
    bool notifications_enabled = true;
    /// 接收文件的绝对目录；相对路径或空值会回退到系统默认下载目录。
    std::string download_directory = Path::DownloadDirectory().toStdString();

    /**
     * @brief 清理下载目录路径，并将无效的空路径或相对路径恢复为默认绝对路径。
     * @param[in,out] config 待规范化的应用程序配置。
     */
    static void normalize(ApplicationConfig &config);

    /**
     * @brief 校验应用程序配置。
     * @param[in] config 待校验的应用程序配置。
     * @return 配置问题列表；空列表表示配置有效。
     */
    [[nodiscard]] static QList<Issue> validate(const ApplicationConfig &config);
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ApplicationConfig, notifications_enabled, download_directory)

} // namespace Config

#endif // APPLICATIONCONFIG_H
