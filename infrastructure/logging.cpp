#include "logging.h"

#include <QDir>
#include <QFileInfo>
#include <QStringList>
#include <QVector>

#include <QsLog.h>
#include <QsLogDest.h>

#include <algorithm>
#include <exception>
#include <limits>
#include <utility>

namespace
{
constexpr int MaxQsLogBackupCount = 10;

QString normalizedLevelName(const std::string &name)
{
    return QString::fromStdString(name).trimmed().toLower();
}

QsLogging::Level configuredLevel(const std::string &name, QsLogging::Level fallback, QStringList *warnings = nullptr)
{
    const QString normalized = normalizedLevelName(name);
    if (normalized == QLatin1String("trace"))
    {
        return QsLogging::TraceLevel;
    }
    if (normalized == QLatin1String("debug"))
    {
        return QsLogging::DebugLevel;
    }
    if (normalized == QLatin1String("info"))
    {
        return QsLogging::InfoLevel;
    }
    if (normalized == QLatin1String("warn") || normalized == QLatin1String("warning"))
    {
        return QsLogging::WarnLevel;
    }
    if (normalized == QLatin1String("error") || normalized == QLatin1String("err"))
    {
        return QsLogging::ErrorLevel;
    }
    if (normalized == QLatin1String("critical") || normalized == QLatin1String("fatal"))
    {
        return QsLogging::FatalLevel;
    }
    if (normalized == QLatin1String("off"))
    {
        return QsLogging::OffLevel;
    }

    if (warnings)
    {
        warnings->append(QStringLiteral("不支持的日志级别“%1”，已使用默认值。").arg(QString::fromStdString(name)));
    }
    return fallback;
}

void installFallbackLogger()
{
    QsLogging::Logger::destroyInstance();
    QsLogging::Logger &logger = QsLogging::Logger::instance();
    logger.setLoggingLevel(QsLogging::InfoLevel);
    logger.setIncludeSourceLocation(false);
    logger.setUseSeparateThread(false);
    logger.addDestination(QsLogging::DestinationFactory::MakeDebugOutputDestination());
}
} // namespace

Config::Result Logging::initialize(const Config::LogConfig &config)
{
    try
    {
        QStringList warnings;
        const QsLogging::Level level = configuredLevel(config.level, QsLogging::InfoLevel, &warnings);
        QVector<QsLogging::DestinationPtr> destinations;

        if (config.console_enabled)
        {
            destinations.append(QsLogging::DestinationFactory::MakeDebugOutputDestination());
        }

        QString logPath;
        if (config.file_enabled)
        {
            logPath = QString::fromStdString(config.file_path);
            const QString directory = QFileInfo(logPath).absolutePath();
            if (!QDir().mkpath(directory))
            {
                installFallbackLogger();
                return Config::Result::failure(QStringLiteral("无法创建日志目录：%1").arg(directory));
            }

            const std::size_t requestedFileSize = std::max<std::size_t>(config.max_file_size, 1);
            const std::size_t requestedFileCount = std::max<std::size_t>(config.max_files, 1);
            const qint64 maxFileSize =
                static_cast<qint64>(std::min<std::size_t>(requestedFileSize, static_cast<std::size_t>(std::numeric_limits<qint64>::max())));
            const int maxFiles = static_cast<int>(std::min<std::size_t>(requestedFileCount, static_cast<std::size_t>(MaxQsLogBackupCount)));
            if (requestedFileSize != config.max_file_size || requestedFileCount != config.max_files)
            {
                warnings.append(QStringLiteral("滚动日志文件限制必须大于零，已使用最小有效值。"));
            }
            if (requestedFileCount > static_cast<std::size_t>(MaxQsLogBackupCount))
            {
                warnings.append(QStringLiteral("QsLog 最多保留 %1 个旧日志文件，已截断配置值。").arg(MaxQsLogBackupCount));
            }

            const QsLogging::DestinationPtr fileDestination = QsLogging::DestinationFactory::MakeFileDestination(
                logPath, QsLogging::EnableLogRotation, QsLogging::MaxSizeBytes(maxFileSize), QsLogging::MaxOldLogCount(maxFiles));
            if (!fileDestination || !fileDestination->isValid())
            {
                installFallbackLogger();
                return Config::Result::failure(QStringLiteral("无法打开日志文件：%1").arg(logPath));
            }
            destinations.append(fileDestination);
        }

        QsLogging::Logger::destroyInstance();
        QsLogging::Logger &logger = QsLogging::Logger::instance();
        logger.setLoggingLevel(level);
        logger.setIncludeSourceLocation(config.source_location_enabled);
        logger.setUseSeparateThread(config.separate_thread_enabled);
        for (const QsLogging::DestinationPtr &destination : std::as_const(destinations))
        {
            logger.addDestination(destination);
        }

        for (const QString &warning : std::as_const(warnings))
        {
            QLOG_WARN() << QStringLiteral("[日志]") << warning;
        }
        QLOG_INFO() << QStringLiteral("[日志] 初始化完成 级别=") << normalizedLevelName(config.level) << QStringLiteral("源码位置=")
                    << config.source_location_enabled << QStringLiteral("独立线程=") << config.separate_thread_enabled << QStringLiteral("控制台=")
                    << config.console_enabled << QStringLiteral("文件=") << config.file_enabled;
        if (!logPath.isEmpty())
        {
            QLOG_DEBUG() << QStringLiteral("[日志] 文件输出路径=") << logPath;
        }
        return {};
    } catch (const std::exception &exception)
    {
        installFallbackLogger();
        return Config::Result::failure(QString::fromUtf8(exception.what()));
    }
}

void Logging::setLevel(const std::string &level)
{
    QsLogging::Logger::instance().setLoggingLevel(configuredLevel(level, QsLogging::InfoLevel));
}

void Logging::shutdown()
{
    QsLogging::Logger::destroyInstance();
}
