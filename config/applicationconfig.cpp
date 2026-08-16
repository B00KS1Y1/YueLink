/**
 * @file applicationconfig.cpp
 * @brief 实现应用程序配置的规范化与校验策略。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-09
 */

#include "applicationconfig.h"

#include "configpolicyutils_p.h"
#include "infrastructure/path.h"

#include <QDir>
#include <QFileInfo>
#include <QString>

namespace Config
{

void ApplicationConfig::normalize(ApplicationConfig &config)
{
    const ApplicationConfig defaults;
    QString downloadDir = QString::fromStdString(config.download_directory).trimmed();
    downloadDir = QDir::fromNativeSeparators(downloadDir);
    if (downloadDir.isEmpty() || !QFileInfo(downloadDir).isAbsolute())
    {
        // 下载位置不解析相对路径，避免其含义随进程工作目录变化。
        downloadDir = QString::fromStdString(defaults.download_directory);
    }
    config.download_directory = QDir::cleanPath(downloadDir).toStdString();
}

QList<Issue> ApplicationConfig::validate(const ApplicationConfig &config)
{
    QList<Issue> issues;
    const QString downloadDir = QString::fromStdString(config.download_directory);
    if (downloadDir.isEmpty() || !QFileInfo(downloadDir).isAbsolute())
    {
        issues.append(Detail::makeIssue(QStringLiteral("download_directory"), QStringLiteral("下载目录必须是绝对路径。")));
    }
    return issues;
}

} // namespace Config
