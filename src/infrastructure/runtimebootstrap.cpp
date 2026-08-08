#include "runtimebootstrap.h"

#include "config/configapi.h"
#include "logging.h"
#include "path.h"

#include <QDebug>
#include <QDir>
#include <QStringList>

#include <spdlog/spdlog.h>

namespace RuntimeBootstrap
{

void initialize()
{
    const QStringList systemDirectories = {Utils::Path::configDirectory(), Utils::Path::logDirectory(), Utils::Path::databaseDirectory()};
    for (const QString &directory : systemDirectories)
    {
        if (!QDir().mkpath(directory))
        {
            qWarning().noquote() << "创建系统目录失败：" << directory;
        }
    }

    const Config::Result configResult = Config::initialize();
    const Config::Result loggingResult = Logging::initialize(Config::get<Config::LogConfig>());
    if (!loggingResult)
    {
        qWarning().noquote() << "初始化日志失败：" << loggingResult.errorMessage;
        spdlog::error("[应用程序] 按配置初始化日志失败 原因={}", loggingResult.errorMessage.toUtf8().toStdString());
    }
    if (!configResult)
    {
        spdlog::warn("[配置] 加载配置失败 原因={}", configResult.errorMessage.toUtf8().toStdString());
    }
}

void shutdown()
{
    Logging::shutdown();
}

} // namespace RuntimeBootstrap
