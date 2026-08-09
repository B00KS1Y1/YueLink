#include "runtimebootstrap.h"

#include "config/configapi.h"
#include "logging.h"
#include "path.h"

#include <QDebug>
#include <QDir>

#include <spdlog/spdlog.h>

namespace RuntimeBootstrap
{

void initialize()
{
    const QString configDirectory = Path::configDirectory();
    if (!QDir().mkpath(configDirectory))
    {
        qWarning().noquote() << "创建配置目录失败：" << configDirectory;
    }

    const Config::Result configResult = Config::initialize(configDirectory);
    const Config::Result loggingResult = Logging::initialize(Config::value<Config::LogConfig>());
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
