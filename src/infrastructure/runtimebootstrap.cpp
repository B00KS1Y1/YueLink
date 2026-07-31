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
    const QStringList systemDirectories = {Utils::Path::configDirectory(), Utils::Path::logDirectory(), Utils::Path::databaseDirectory()};
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
        qWarning().noquote() << "初始化日志失败：" << loggingResult.errorMessage;
        spdlog::error("[应用程序] 按配置初始化日志失败 原因={}", loggingResult.errorMessage.toUtf8().toStdString());
    }
    if (!logConfigResult)
    {
        spdlog::warn("[配置] 加载日志配置失败 原因={}", logConfigResult.errorMessage.toUtf8().toStdString());
    }

    const auto loadConfig = [](const char *name, auto &store) {
        const Config::Result result = store.load();
        if (!result)
        {
            spdlog::warn("[配置] 加载{}配置失败 原因={}", name, result.errorMessage.toUtf8().toStdString());
        }
    };
    loadConfig("主题", Config::theme);
    const Config::Result applicationConfigResult = Config::application.load();
    if (!applicationConfigResult)
    {
        spdlog::warn("[配置] 加载应用程序配置失败 原因={}", applicationConfigResult.errorMessage.toUtf8().toStdString());
    }
    else if (!QDir::isAbsolutePath(QString::fromStdString(Config::application.get().download_directory).trimmed()))
    {
        Config::ApplicationConfig application = Config::application.get();
        application.download_directory = Utils::Path::defaultDownloadDirectory().toStdString();
        Config::application.set(application);
        const Config::Result saveResult = Config::application.save();
        if (!saveResult)
        {
            spdlog::warn("[配置] 保存默认下载目录失败 原因={}", saveResult.errorMessage.toUtf8().toStdString());
        }
    }
    loadConfig("数据库", Config::database);
}

void shutdown()
{
    Logging::shutdown();
}

} // namespace RuntimeBootstrap
