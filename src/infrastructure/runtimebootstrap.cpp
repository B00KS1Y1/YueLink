#include "runtimebootstrap.h"

#include "config/configstore.h"
#include "logging.h"

#include <QDebug>

#include <spdlog/spdlog.h>

namespace RuntimeBootstrap
{

void initialize()
{
    const Config::Result logConfigResult = Config::log.load();
    const Config::Result loggingResult = Logging::initialize(Config::log.get());
    if (!loggingResult)
    {
        qWarning().noquote() << "Failed to initialize logging:"
                             << loggingResult.errorMessage;
        spdlog::error("[application] failed to initialize configured logging: {}",
                      loggingResult.errorMessage.toUtf8().toStdString());
    }
    if (!logConfigResult)
    {
        spdlog::warn("[configuration] failed to load log configuration: {}",
                     logConfigResult.errorMessage.toUtf8().toStdString());
    }

    const auto loadConfig = [](const char *name, auto &store) {
        const Config::Result result = store.load();
        if (!result)
        {
            spdlog::warn("[configuration] failed to load {} configuration: {}",
                         name,
                         result.errorMessage.toUtf8().toStdString());
        }
    };
    loadConfig("theme", Config::theme);
    loadConfig("application", Config::application);
    loadConfig("database", Config::database);
}

void shutdown()
{
    Logging::shutdown();
}

} // namespace RuntimeBootstrap
