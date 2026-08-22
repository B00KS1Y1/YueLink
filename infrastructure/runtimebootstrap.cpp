#include "runtimebootstrap.h"

#include "config/configapi.h"
#include "logging.h"
#include "path.h"

#include <QDir>

#include <QyLog.h>

namespace RuntimeBootstrap
{

void initialize()
{
    const QString configDirectory = Path::configDirectory();
    QString directoryError;
    if (!QDir().mkpath(configDirectory))
    {
        directoryError = QStringLiteral("创建配置目录失败：%1").arg(configDirectory);
    }

    const Config::Result configResult = Config::initialize(configDirectory);
    const Config::Result loggingResult = Logging::initialize(Config::value<Config::LogConfig>());
    if (!directoryError.isEmpty())
    {
        QLOG_ERROR() << QStringLiteral("[配置]") << directoryError;
    }
    if (!loggingResult)
    {
        QLOG_ERROR() << QStringLiteral("[应用程序] 按配置初始化日志失败 原因=") << loggingResult.errorMessage;
    }
    if (!configResult)
    {
        QLOG_WARN() << QStringLiteral("[配置] 加载配置失败 原因=") << configResult.errorMessage;
    }
}

void shutdown()
{
    Logging::shutdown();
}

} // namespace RuntimeBootstrap
