#include "runtimebootstrap.h"

#include "config/configstore.h"
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
    const QStringList systemDirectories = {
        Utils::Path::configDirectory(),
        Utils::Path::logDirectory(),
        Utils::Path::databaseDirectory()
    };
    for (const QString &directory : systemDirectories)
    {
        if (!QDir().mkpath(directory))
        {
            qWarning().noquote() << "创建系统目录失败：" << directory;
        }
    }

    const Config::Result logConfigResult = Config::log.load();
    const Config::Result loggingResult = Logging::initialize(Config::log.get());
    if (!loggingResult)
    {
        qWarning().noquote() << "初始化日志失败："
                             << loggingResult.errorMessage;
        spdlog::error("[应用程序] 按配置初始化日志失败 原因={}",
                      loggingResult.errorMessage.toUtf8().toStdString());
    }
    if (!logConfigResult)
    {
        spdlog::warn("[配置] 加载日志配置失败 原因={}",
                     logConfigResult.errorMessage.toUtf8().toStdString());
    }

    const auto loadConfig = [](const char *name, auto &store) {
        const Config::Result result = store.load();
        if (!result)
        {
            spdlog::warn("[配置] 加载{}配置失败 原因={}",
                         name,
                         result.errorMessage.toUtf8().toStdString());
        }
    };
    loadConfig("主题", Config::theme);
    loadConfig("应用程序", Config::application);
    loadConfig("数据库", Config::database);
}

void shutdown()
{
    Logging::shutdown();
}

} // namespace RuntimeBootstrap
