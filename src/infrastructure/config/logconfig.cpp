/**
 * @file logconfig.cpp
 * @brief 实现日志配置的规范化与校验策略。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-09
 */

#include "logconfig.h"

#include "configpolicyutils_p.h"
#include "infrastructure/path.h"

#include <QDir>
#include <QFileInfo>
#include <QSet>
#include <QString>

namespace
{
std::string normalizedLevel(const std::string &value)
{
    QString level = Config::Detail::normalizedName(value);
    // 接受 spdlog 常见别名，但持久化时统一使用标准级别名称。
    if (level == QLatin1String("warning"))
    {
        level = QStringLiteral("warn");
    }
    else if (level == QLatin1String("err"))
    {
        level = QStringLiteral("error");
    }
    return level.toStdString();
}

bool isSupportedLogLevel(const std::string &value)
{
    static const QSet<QString> levels = {QStringLiteral("trace"),
                                         QStringLiteral("debug"),
                                         QStringLiteral("info"),
                                         QStringLiteral("warn"),
                                         QStringLiteral("error"),
                                         QStringLiteral("critical"),
                                         QStringLiteral("off")};
    return levels.contains(QString::fromStdString(value));
}
} // namespace

namespace Config
{

void LogConfig::normalize(LogConfig &config)
{
    config.level = normalizedLevel(config.level);
    config.flush_level = normalizedLevel(config.flush_level);
    if (config.pattern.empty())
    {
        config.pattern = LogConfig{}.pattern;
    }
    QString path = QString::fromStdString(config.file_path).trimmed();
    if (path.isEmpty())
    {
        path = QString::fromStdString(LogConfig{}.file_path);
    }
    path = QDir::fromNativeSeparators(path);
    // 日志相对路径固定锚定在应用日志目录，绝对路径则尊重用户配置。
    config.file_path = QFileInfo(path).isAbsolute() ? QDir::cleanPath(path).toStdString() : Path::logFile(path).toStdString();
}

QList<Issue> LogConfig::validate(const LogConfig &config)
{
    QList<Issue> issues;
    if (!isSupportedLogLevel(config.level))
    {
        issues.append(Detail::makeIssue(QStringLiteral("level"), QStringLiteral("日志级别无效。")));
    }
    if (!isSupportedLogLevel(config.flush_level))
    {
        issues.append(Detail::makeIssue(QStringLiteral("flush_level"), QStringLiteral("刷新级别无效。")));
    }
    if (config.file_enabled && !QFileInfo(QString::fromStdString(config.file_path)).isAbsolute())
    {
        issues.append(Detail::makeIssue(QStringLiteral("file_path"), QStringLiteral("启用文件日志时必须使用绝对路径。")));
    }
    if (config.file_enabled && config.max_file_size == 0)
    {
        issues.append(Detail::makeIssue(QStringLiteral("max_file_size"), QStringLiteral("滚动日志文件大小必须大于零。")));
    }
    if (config.file_enabled && config.max_files == 0)
    {
        issues.append(Detail::makeIssue(QStringLiteral("max_files"), QStringLiteral("滚动日志文件数量必须大于零。")));
    }
    if (config.async && config.async_queue_size == 0)
    {
        issues.append(Detail::makeIssue(QStringLiteral("async_queue_size"), QStringLiteral("异步队列大小必须大于零。")));
    }
    if (config.async && config.async_thread_count == 0)
    {
        issues.append(Detail::makeIssue(QStringLiteral("async_thread_count"), QStringLiteral("异步线程数必须大于零。")));
    }
    return issues;
}

} // namespace Config
