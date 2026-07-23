#include "desktopfilelauncher.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QUrl>

#include <spdlog/spdlog.h>

namespace
{
bool validatePath(const QString &filePath,
                  QFileInfo *fileInfo,
                  QString *errorMessage)
{
    const QFileInfo info(QDir::cleanPath(filePath));
    if (filePath.trimmed().isEmpty() || !info.exists())
    {
        if (errorMessage)
        {
            *errorMessage = QStringLiteral("文件不存在或已经被移动。");
        }
        return false;
    }
    if (fileInfo)
    {
        *fileInfo = info;
    }
    return true;
}

void clearError(QString *errorMessage)
{
    if (errorMessage)
    {
        errorMessage->clear();
    }
}
} // namespace

bool DesktopFileLauncher::openFile(const QString &filePath,
                                   QString *errorMessage)
{
    QFileInfo fileInfo;
    if (!validatePath(filePath, &fileInfo, errorMessage))
    {
        return false;
    }
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absoluteFilePath())))
    {
        if (errorMessage)
        {
            *errorMessage = QStringLiteral("系统无法打开该文件。");
        }
        spdlog::warn("[platform.file] open file failed path={}",
                     fileInfo.absoluteFilePath().toUtf8().toStdString());
        return false;
    }
    clearError(errorMessage);
    spdlog::debug("[platform.file] opened file path={}",
                  fileInfo.absoluteFilePath().toUtf8().toStdString());
    return true;
}

bool DesktopFileLauncher::revealInFolder(const QString &filePath,
                                         QString *errorMessage)
{
    QFileInfo fileInfo;
    if (!validatePath(filePath, &fileInfo, errorMessage))
    {
        return false;
    }

    bool started = false;
#if defined(Q_OS_WIN)
    started = QProcess::startDetached(
        QStringLiteral("explorer.exe"),
        {QStringLiteral("/select,"),
         QDir::toNativeSeparators(fileInfo.absoluteFilePath())});
#elif defined(Q_OS_MACOS)
    started = QProcess::startDetached(QStringLiteral("open"),
                                      {QStringLiteral("-R"),
                                       fileInfo.absoluteFilePath()});
#else
    started = QDesktopServices::openUrl(
        QUrl::fromLocalFile(fileInfo.absolutePath()));
#endif

    if (!started)
    {
        if (errorMessage)
        {
            *errorMessage = QStringLiteral("系统无法打开文件所在目录。");
        }
        spdlog::warn("[platform.file] reveal file failed path={}",
                     fileInfo.absoluteFilePath().toUtf8().toStdString());
        return false;
    }
    clearError(errorMessage);
    spdlog::debug("[platform.file] revealed file path={}",
                  fileInfo.absoluteFilePath().toUtf8().toStdString());
    return true;
}
