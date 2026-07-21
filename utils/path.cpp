#include "path.h"

#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>

namespace Utils::Path
{

QString configDirectory()
{
    QString directory = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if (directory.isEmpty())
    {
        directory = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("config"));
    }
    return QDir::cleanPath(directory);
}

QString configFile(const QString &fileName)
{
    return QDir(configDirectory()).filePath(fileName);
}

QString dataDirectory()
{
    QString directory = QStandardPaths::writableLocation(
        QStandardPaths::AppLocalDataLocation);
    if (directory.isEmpty())
    {
        directory = QDir(QCoreApplication::applicationDirPath())
                        .filePath(QStringLiteral("data"));
    }
    return QDir::cleanPath(directory);
}

QString dataFile(const QString &fileName)
{
    return QDir(dataDirectory()).filePath(fileName);
}

} // namespace Utils::Path
