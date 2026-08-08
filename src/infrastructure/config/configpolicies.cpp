#include "applicationconfig.h"
#include "databaseconfig.h"
#include "logconfig.h"
#include "themeconfig.h"

#include "infrastructure/path.h"

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSet>
#include <QString>

#include <utility>

namespace
{
Config::Issue issue(QString fieldPath, QString message)
{
    return {std::move(fieldPath), std::move(message)};
}

QString normalizedName(const std::string &value)
{
    return QString::fromStdString(value).trimmed().toLower();
}

std::string normalizedLevel(const std::string &value)
{
    QString level = normalizedName(value);
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

bool isColor(const std::string &value, bool allowEmpty = false)
{
    const QString color = QString::fromStdString(value);
    if (allowEmpty && color.isEmpty())
    {
        return true;
    }
    static const QRegularExpression expression(QStringLiteral("^#[0-9A-F]{6}$"));
    return expression.match(color).hasMatch();
}

void normalizeColor(std::string &value)
{
    value = QString::fromStdString(value).trimmed().toUpper().toStdString();
}
} // namespace

namespace Config
{

void ApplicationConfig::normalize(ApplicationConfig &config)
{
    const ApplicationConfig defaults;
    QString downloadDir = QString::fromStdString(config.download_directory).trimmed();
    downloadDir = QDir::fromNativeSeparators(downloadDir);
    if (downloadDir.isEmpty() || !QFileInfo(downloadDir).isAbsolute())
    {
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
        issues.append(issue(QStringLiteral("download_directory"), QStringLiteral("下载目录必须是绝对路径。")));
    }

    return issues;
}

void ThemeConfig::normalize(ThemeConfig &config)
{
    config.mode = normalizedName(config.mode).toStdString();
    config.navigation_mode = normalizedName(config.navigation_mode).toStdString();
    normalizeColor(config.primary_color);
    normalizeColor(config.dark_background);
    normalizeColor(config.light_background);
}

QList<Issue> ThemeConfig::validate(const ThemeConfig &config)
{
    QList<Issue> issues;
    static const QSet<QString> themeModes = {QStringLiteral("light"), QStringLiteral("dark"), QStringLiteral("system")};
    static const QSet<QString> navigationModes = {QStringLiteral("relaxed"), QStringLiteral("standard"), QStringLiteral("compact")};
    if (!themeModes.contains(QString::fromStdString(config.mode)))
    {
        issues.append(issue(QStringLiteral("mode"), QStringLiteral("仅支持 light、dark 或 system。")));
    }
    if (!navigationModes.contains(QString::fromStdString(config.navigation_mode)))
    {
        issues.append(issue(QStringLiteral("navigation_mode"), QStringLiteral("仅支持 relaxed、standard 或 compact。")));
    }
    if (!isColor(config.primary_color))
    {
        issues.append(issue(QStringLiteral("primary_color"), QStringLiteral("颜色必须使用 #RRGGBB 格式。")));
    }
    if (!isColor(config.dark_background, true))
    {
        issues.append(issue(QStringLiteral("dark_background"), QStringLiteral("颜色必须为空或使用 #RRGGBB 格式。")));
    }
    if (!isColor(config.light_background, true))
    {
        issues.append(issue(QStringLiteral("light_background"), QStringLiteral("颜色必须为空或使用 #RRGGBB 格式。")));
    }
    return issues;
}

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
    config.file_path = QFileInfo(path).isAbsolute() ? QDir::cleanPath(path).toStdString()
                                                   : Path::logFile(path).toStdString();
}

QList<Issue> LogConfig::validate(const LogConfig &config)
{
    QList<Issue> issues;
    if (!isSupportedLogLevel(config.level))
    {
        issues.append(issue(QStringLiteral("level"), QStringLiteral("日志级别无效。")));
    }
    if (!isSupportedLogLevel(config.flush_level))
    {
        issues.append(issue(QStringLiteral("flush_level"), QStringLiteral("刷新级别无效。")));
    }
    if (config.file_enabled && !QFileInfo(QString::fromStdString(config.file_path)).isAbsolute())
    {
        issues.append(issue(QStringLiteral("file_path"), QStringLiteral("启用文件日志时必须使用绝对路径。")));
    }
    if (config.file_enabled && config.max_file_size == 0)
    {
        issues.append(issue(QStringLiteral("max_file_size"), QStringLiteral("滚动日志文件大小必须大于零。")));
    }
    if (config.file_enabled && config.max_files == 0)
    {
        issues.append(issue(QStringLiteral("max_files"), QStringLiteral("滚动日志文件数量必须大于零。")));
    }
    if (config.async && config.async_queue_size == 0)
    {
        issues.append(issue(QStringLiteral("async_queue_size"), QStringLiteral("异步队列大小必须大于零。")));
    }
    if (config.async && config.async_thread_count == 0)
    {
        issues.append(issue(QStringLiteral("async_thread_count"), QStringLiteral("异步线程数必须大于零。")));
    }
    return issues;
}

void DatabaseConfig::normalize(DatabaseConfig &config)
{
    config.driver = normalizedName(config.driver).toStdString();
    QString sqlitePath = QString::fromStdString(config.sqlite.file_path).trimmed();
    if (sqlitePath.isEmpty())
    {
        sqlitePath = QString::fromStdString(SqliteConfig{}.file_path);
    }
    sqlitePath = QDir::fromNativeSeparators(sqlitePath);
    config.sqlite.file_path = QFileInfo(sqlitePath).isAbsolute() ? QDir::cleanPath(sqlitePath).toStdString()
                                                                 : Path::databaseFile(sqlitePath).toStdString();
    config.sqlite.synchronous = normalizedName(config.sqlite.synchronous).toStdString();
    config.mysql.host = QString::fromStdString(config.mysql.host).trimmed().toStdString();
    config.mysql.database = QString::fromStdString(config.mysql.database).trimmed().toStdString();
    config.mysql.username = QString::fromStdString(config.mysql.username).trimmed().toStdString();
    config.mysql.charset = QString::fromStdString(config.mysql.charset).trimmed().toStdString();
}

QList<Issue> DatabaseConfig::validate(const DatabaseConfig &config)
{
    QList<Issue> issues;
    if (config.driver != "sqlite")
    {
        issues.append(issue(QStringLiteral("driver"), QStringLiteral("当前版本仅支持 sqlite。")));
    }
    if (!QFileInfo(QString::fromStdString(config.sqlite.file_path)).isAbsolute())
    {
        issues.append(issue(QStringLiteral("sqlite.file_path"), QStringLiteral("SQLite 数据库必须使用绝对路径。")));
    }
    if (config.sqlite.pool_size == 0)
    {
        issues.append(issue(QStringLiteral("sqlite.pool_size"), QStringLiteral("连接池大小必须大于零。")));
    }
    if (config.sqlite.busy_timeout_ms == 0 || config.sqlite.busy_timeout_ms > 600000)
    {
        issues.append(issue(QStringLiteral("sqlite.busy_timeout_ms"), QStringLiteral("忙等待超时必须在 1 到 600000 毫秒之间。")));
    }
    static const QSet<QString> synchronousModes = {QStringLiteral("off"), QStringLiteral("normal"), QStringLiteral("full"), QStringLiteral("extra")};
    if (!synchronousModes.contains(QString::fromStdString(config.sqlite.synchronous)))
    {
        issues.append(issue(QStringLiteral("sqlite.synchronous"), QStringLiteral("SQLite 同步模式无效。")));
    }
    return issues;
}

} // namespace Config
