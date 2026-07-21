#include <QApplication>
#include <QQmlApplicationEngine>
#include <QDebug>

#include "config/configstore.h"
#include "logging/logging.h"

#include <spdlog/spdlog.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("YueLink"));
    QCoreApplication::setApplicationName(QStringLiteral("YueLink"));

    const Config::Result logConfigResult = Config::log.load();
    const Config::Result loggingResult = Logging::initialize(Config::log.get());
    if (!loggingResult)
    {
        qWarning().noquote() << "Failed to initialize logging:" << loggingResult.errorMessage;
        spdlog::error("[application] failed to initialize configured logging: {}", loggingResult.errorMessage.toUtf8().toStdString());
    }
    if (!logConfigResult)
    {
        spdlog::warn("[configuration] failed to load log configuration: {}", logConfigResult.errorMessage.toUtf8().toStdString());
    }

    const auto loadConfig = [](const char *name, auto &store) {
        const Config::Result result = store.load();
        if (!result)
        {
            spdlog::warn("[configuration] failed to load {} configuration: {}", name, result.errorMessage.toUtf8().toStdString());
        }
    };
    loadConfig("theme", Config::theme);
    loadConfig("application", Config::application);
    loadConfig("database", Config::database);
    spdlog::info("[application] YueLink starting");

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() {
            spdlog::critical("[application] QML root object creation failed");
            QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.loadFromModule("YueLink", "Main");

    const int exitCode = QCoreApplication::exec();
    spdlog::info("[application] YueLink stopped exit_code={}", exitCode);
    Logging::shutdown();
    return exitCode;
}
