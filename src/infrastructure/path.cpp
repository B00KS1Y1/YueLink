#include "path.h"

#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
#include <QtGlobal>

namespace
{
QString configuredDirectory(const char *environmentVariable, const QString &baseDirectory, const QString &defaultRelativePath)
{
    QString path = qEnvironmentVariable(environmentVariable).trimmed();
    if (path.isEmpty())
    {
        path = defaultRelativePath;
    }
    return QDir::isAbsolutePath(path) ? QDir::cleanPath(path) : QDir::cleanPath(QDir(baseDirectory).filePath(path));
}

QString withoutLeadingDirectory(const QString &filePath, const QString &directoryName)
{
    const QString normalizedPath = QDir::cleanPath(QDir::fromNativeSeparators(filePath));
    const QString prefix = directoryName + QLatin1Char('/');
    return normalizedPath.startsWith(prefix, Qt::CaseInsensitive) ? normalizedPath.mid(prefix.size()) : normalizedPath;
}
} // namespace

namespace Utils::Path
{

QString dataDirectory()
{
    QString directory = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if (directory.isEmpty())
    {
        directory = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("data"));
    }
    return QDir::cleanPath(directory);
}

QString dataFile(const QString &fileName)
{
    return QDir::cleanPath(QDir(dataDirectory()).filePath(fileName));
}

QString systemDirectory()
{
    return configuredDirectory("YUELINK_SYSTEM_DIR", QCoreApplication::applicationDirPath(), QStringLiteral("system"));
}

QString configDirectory()
{
    return configuredDirectory("YUELINK_CONFIG_DIR", systemDirectory(), QStringLiteral("configs"));
}

QString configFile(const QString &fileName)
{
    return QDir::cleanPath(QDir(configDirectory()).filePath(fileName));
}

QString logDirectory()
{
    return configuredDirectory("YUELINK_LOG_DIR", systemDirectory(), QStringLiteral("logs"));
}

QString logFile(const QString &fileName)
{
    const QString relativePath = withoutLeadingDirectory(fileName, QStringLiteral("logs"));
    return QDir::cleanPath(QDir(logDirectory()).filePath(relativePath));
}

QString databaseDirectory()
{
    return configuredDirectory("YUELINK_DATABASE_DIR", systemDirectory(), QStringLiteral("database"));
}

QString databaseFile(const QString &fileName)
{
    QString relativePath = withoutLeadingDirectory(fileName, QStringLiteral("data"));
    relativePath = withoutLeadingDirectory(relativePath, QStringLiteral("database"));
    return QDir::cleanPath(QDir(databaseDirectory()).filePath(relativePath));
}

} // namespace Utils::Path
