#include "logging.h"

#include "path.h"

#include <QDir>
#include <QFileInfo>

#include <spdlog/async.h>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <chrono>
#include <exception>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace
{
std::string normalizedLevelName(const std::string &name)
{
    QString normalized = QString::fromStdString(name).trimmed().toLower();
    return normalized.toStdString();
}

spdlog::level::level_enum configuredLevel(const std::string &name, spdlog::level::level_enum fallback, std::vector<std::string> &warnings)
{
    const std::string normalized = normalizedLevelName(name);
    const spdlog::level::level_enum level = spdlog::level::from_str(normalized);
    if (level != spdlog::level::off || normalized == "off")
    {
        return level;
    }

    warnings.push_back("不支持的日志级别 '" + name + "'，已使用默认值");
    return fallback;
}

QString resolvedLogPath(const std::string &configuredPath)
{
    const QString path = QString::fromStdString(configuredPath).trimmed();
    if (path.isEmpty() || QFileInfo(path).isAbsolute())
    {
        return QDir::cleanPath(path);
    }

    return QDir::cleanPath(Utils::Path::logFile(path));
}

spdlog::filename_t nativeLogPath(const QString &path)
{
#ifdef SPDLOG_WCHAR_FILENAMES
    return path.toStdWString();
#else
    return path.toUtf8().toStdString();
#endif
}

void installFallbackLogger()
{
    spdlog::drop_all();
    auto sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
    auto logger = std::make_shared<spdlog::logger>("YueLink", std::move(sink));
    logger->set_level(spdlog::level::info);
    logger->flush_on(spdlog::level::warn);
    spdlog::set_default_logger(std::move(logger));
}
} // namespace

Config::Result Logging::initialize(const Config::LogConfig &config)
{
    try
    {
        std::vector<std::string> warnings;
        const spdlog::level::level_enum level = configuredLevel(config.level, spdlog::level::info, warnings);
        const spdlog::level::level_enum flushLevel = configuredLevel(config.flush_level, spdlog::level::warn, warnings);

        std::vector<spdlog::sink_ptr> sinks;
        if (config.console_enabled)
        {
            if (config.console_color)
            {
                sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
            }
            else
            {
                sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_mt>());
            }
        }

        QString logPath;
        if (config.file_enabled)
        {
            logPath = resolvedLogPath(config.file_path);
            if (logPath.isEmpty())
            {
                throw spdlog::spdlog_ex("日志文件路径为空");
            }
            const QString directory = QFileInfo(logPath).absolutePath();
            if (!QDir().mkpath(directory))
            {
                throw spdlog::spdlog_ex("无法创建日志目录：" + directory.toUtf8().toStdString());
            }

            const std::size_t maxFileSize = std::max<std::size_t>(config.max_file_size, 1);
            const std::size_t maxFiles = std::max<std::size_t>(config.max_files, 1);
            if (maxFileSize != config.max_file_size || maxFiles != config.max_files)
            {
                warnings.push_back("滚动日志文件限制必须大于零，已使用默认值");
            }
            sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(nativeLogPath(logPath), maxFileSize, maxFiles, config.rotate_on_open));
        }
        if (sinks.empty())
        {
            sinks.push_back(std::make_shared<spdlog::sinks::null_sink_mt>());
        }

        std::shared_ptr<spdlog::logger> logger;
        if (config.async)
        {
            const std::size_t queueSize = std::max<std::size_t>(config.async_queue_size, 1);
            const std::size_t threadCount = std::max<std::size_t>(config.async_thread_count, 1);
            if (queueSize != config.async_queue_size || threadCount != config.async_thread_count)
            {
                warnings.push_back("异步队列大小和线程数必须大于零，已使用默认值");
            }
            spdlog::init_thread_pool(queueSize, threadCount);
            logger = std::make_shared<spdlog::async_logger>("YueLink", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
        }
        else
        {
            logger = std::make_shared<spdlog::logger>("YueLink", sinks.begin(), sinks.end());
        }

        logger->set_pattern(config.pattern.empty() ? Config::LogConfig{}.pattern : config.pattern);
        logger->set_level(level);
        logger->flush_on(flushLevel);
        spdlog::set_default_logger(std::move(logger));
        if (config.flush_every_seconds > 0)
        {
            spdlog::flush_every(std::chrono::seconds(config.flush_every_seconds));
        }

        for (const std::string &warning : warnings)
        {
            spdlog::warn("[日志] {}", warning);
        }
        spdlog::info("[日志] 初始化完成 级别={} 控制台={} 文件={} 异步={}",
                     normalizedLevelName(config.level),
                     config.console_enabled,
                     config.file_enabled,
                     config.async);
        if (!logPath.isEmpty())
        {
            spdlog::debug("[日志] 文件输出路径={}", logPath.toUtf8().toStdString());
        }
        return {};
    } catch (const std::exception &exception)
    {
        installFallbackLogger();
        return Config::Result::failure(QString::fromUtf8(exception.what()));
    }
}

void Logging::shutdown()
{
    spdlog::shutdown();
}
