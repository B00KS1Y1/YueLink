/**
 * @file databaseconfig.cpp
 * @brief 实现数据库配置的规范化与校验策略。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-09
 */

#include "databaseconfig.h"

#include "configpolicyutils_p.h"
#include "infrastructure/path.h"

#include <QDir>
#include <QFileInfo>
#include <QSet>
#include <QString>

namespace Config
{

void DatabaseConfig::normalize(DatabaseConfig &config)
{
    config.driver = Detail::normalizedName(config.driver).toStdString();
    QString sqlitePath = QString::fromStdString(config.sqlite.file_path).trimmed();
    if (sqlitePath.isEmpty())
    {
        sqlitePath = QString::fromStdString(SqliteConfig{}.file_path);
    }
    sqlitePath = QDir::fromNativeSeparators(sqlitePath);
    // 数据库相对路径固定锚定在应用数据库目录，并兼容旧版目录前缀。
    config.sqlite.file_path =
        QFileInfo(sqlitePath).isAbsolute() ? QDir::cleanPath(sqlitePath).toStdString() : Path::databaseFile(sqlitePath).toStdString();
    config.sqlite.synchronous = Detail::normalizedName(config.sqlite.synchronous).toStdString();
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
        issues.append(Detail::makeIssue(QStringLiteral("driver"), QStringLiteral("当前版本仅支持 sqlite。")));
    }
    if (!QFileInfo(QString::fromStdString(config.sqlite.file_path)).isAbsolute())
    {
        issues.append(Detail::makeIssue(QStringLiteral("sqlite.file_path"), QStringLiteral("SQLite 数据库必须使用绝对路径。")));
    }
    if (config.sqlite.pool_size == 0)
    {
        issues.append(Detail::makeIssue(QStringLiteral("sqlite.pool_size"), QStringLiteral("连接池大小必须大于零。")));
    }
    if (config.sqlite.busy_timeout_ms == 0 || config.sqlite.busy_timeout_ms > 600000)
    {
        issues.append(Detail::makeIssue(QStringLiteral("sqlite.busy_timeout_ms"), QStringLiteral("忙等待超时必须在 1 到 600000 毫秒之间。")));
    }
    static const QSet<QString> synchronousModes = {QStringLiteral("off"), QStringLiteral("normal"), QStringLiteral("full"), QStringLiteral("extra")};
    if (!synchronousModes.contains(QString::fromStdString(config.sqlite.synchronous)))
    {
        issues.append(Detail::makeIssue(QStringLiteral("sqlite.synchronous"), QStringLiteral("SQLite 同步模式无效。")));
    }
    return issues;
}

} // namespace Config
