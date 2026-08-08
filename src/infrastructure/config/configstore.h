/**
 * @file configstore.h
 * @brief 定义具有自动映射、校验和原子写入能力的类型化配置存储。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-06
 */

#ifndef CONFIGSTORE_H
#define CONFIGSTORE_H

#include "configbase.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QReadLocker>
#include <QReadWriteLock>
#include <QSaveFile>
#include <QString>
#include <QWriteLocker>

#include <nlohmann/json.hpp>

#include <exception>
#include <functional>
#include <type_traits>
#include <utility>

namespace Config
{

template <typename T> struct Snapshot
{
    T value;
    quint64 revision = 0;
};

template <typename T> class ConfigStore
{
    static_assert(std::is_base_of_v<ConfigBase<T>, T>, "Configuration types must derive from ConfigBase<T>.");

public:
    using ValueType = T;

    /** @brief 构造尚未初始化的配置存储。 */
    ConfigStore() = default;

    /**
     * @brief 使用指定目录初始化并加载配置。
     * @param[in] configDirectory 配置文件所在目录。
     * @return 初始化和加载结果；失败时保留默认配置。
     */
    [[nodiscard]] Result initialize(const QString &configDirectory)
    {
        QWriteLocker locker(&m_lock);
        const bool firstLoad = !m_initialized;
        m_filePath = QDir::cleanPath(QDir(configDirectory).filePath(QString::fromLatin1(T::FileName)));
        if (!m_initialized)
        {
            m_config = T::defaults();
            T::normalize(m_config);
            m_initialized = true;
        }
        return loadLocked(firstLoad);
    }

    /**
     * @brief 返回当前配置副本。
     * @return 当前内存配置。
     */
    [[nodiscard]] T get() const
    {
        QReadLocker locker(&m_lock);
        return m_config;
    }

    /**
     * @brief 返回带修订号的当前配置快照。
     * @return 当前配置和修订号。
     */
    [[nodiscard]] Snapshot<T> snapshot() const
    {
        QReadLocker locker(&m_lock);
        return {m_config, m_revision};
    }

    /**
     * @brief 校验并原子保存新的完整配置。
     * @param[in] candidate 候选配置。
     * @return 保存结果；失败时内存和原文件均保持不变。
     */
    [[nodiscard]] Result set(T candidate)
    {
        QWriteLocker locker(&m_lock);
        return setLocked(std::move(candidate), true);
    }

    /**
     * @brief 在当前配置副本上执行修改并原子保存。
     * @tparam Mutator 可调用修改器类型。
     * @param[in] mutator 接收 @c T& 的配置修改器。
     * @return 更新结果；修改器或保存失败时当前配置保持不变。
     */
    template <typename Mutator> [[nodiscard]] Result update(Mutator &&mutator)
    {
        T candidate;
        quint64 baseRevision = 0;
        QString operationFilePath;
        {
            QReadLocker locker(&m_lock);
            if (!m_initialized)
            {
                return Result::failure(ErrorCode::NotInitialized, QStringLiteral("配置存储尚未初始化。"), m_filePath);
            }
            candidate = m_config;
            baseRevision = m_revision;
            operationFilePath = m_filePath;
        }

        try
        {
            std::invoke(std::forward<Mutator>(mutator), candidate);
        } catch (const std::exception &exception)
        {
            return Result::failure(ErrorCode::MutationFailed, QStringLiteral("修改配置失败：%1").arg(QString::fromUtf8(exception.what())), operationFilePath);
        } catch (...)
        {
            return Result::failure(ErrorCode::MutationFailed, QStringLiteral("修改配置时发生未知异常。"), operationFilePath);
        }

        QWriteLocker locker(&m_lock);
        if (m_revision != baseRevision)
        {
            return Result::failure(ErrorCode::Conflict, QStringLiteral("配置在修改期间已被其他操作更新，请重试。"), m_filePath);
        }
        return setLocked(std::move(candidate), false);
    }

    /**
     * @brief 从当前配置文件重新加载数据。
     * @return 重新加载结果；失败时保留当前内存配置。
     */
    [[nodiscard]] Result reload()
    {
        QWriteLocker locker(&m_lock);
        if (!m_initialized)
        {
            return Result::failure(ErrorCode::NotInitialized, QStringLiteral("配置存储尚未初始化。"), m_filePath);
        }
        return loadLocked(false);
    }

    /**
     * @brief 恢复默认配置并原子保存。
     * @return 重置结果。
     */
    [[nodiscard]] Result reset()
    {
        QWriteLocker locker(&m_lock);
        if (!m_initialized)
        {
            return Result::failure(ErrorCode::NotInitialized, QStringLiteral("配置存储尚未初始化。"), m_filePath);
        }
        T defaults = T::defaults();
        return setLocked(std::move(defaults), true);
    }

    /**
     * @brief 返回该配置对应的绝对文件路径。
     * @return 配置文件路径；初始化前返回空字符串。
     */
    [[nodiscard]] QString filePath() const
    {
        QReadLocker locker(&m_lock);
        return m_filePath;
    }

private:
    /**
     * @brief 在持有写锁时加载配置。
     * @param[in] firstLoad 是否为本存储的首次加载。
     * @return 加载结果；文件不存在或缺少字段时使用默认值重新生成配置文件。
     */
    [[nodiscard]] Result loadLocked(bool firstLoad)
    {
        const QFileInfo info(m_filePath);
        if (!info.exists())
        {
            T defaults = T::defaults();
            T::normalize(defaults);
            const QList<Issue> issues = T::validate(defaults);
            if (!issues.isEmpty())
            {
                return Result::validationFailure(issues, m_filePath);
            }
            try
            {
                const Result saveResult = saveCandidateLocked(nlohmann::json(defaults));
                if (!saveResult)
                {
                    return saveResult;
                }
            } catch (const std::exception &exception)
            {
                return Result::failure(
                    ErrorCode::SerializationFailed, QStringLiteral("序列化默认配置失败：%1").arg(QString::fromUtf8(exception.what())), m_filePath);
            }
            return publishLoaded(std::move(defaults), firstLoad);
        }
        if (!info.isFile())
        {
            return Result::failure(ErrorCode::InvalidPath, QStringLiteral("配置路径不是常规文件。"), m_filePath);
        }

        QFile file(m_filePath);
        if (!file.open(QIODevice::ReadOnly))
        {
            return Result::failure(ErrorCode::OpenFailed, QStringLiteral("打开配置文件失败：%1").arg(file.errorString()), m_filePath);
        }
        const QByteArray bytes = file.readAll();
        if (file.error() != QFileDevice::NoError)
        {
            return Result::failure(ErrorCode::ReadFailed, QStringLiteral("读取配置文件失败：%1").arg(file.errorString()), m_filePath);
        }

        try
        {
            nlohmann::json json = nlohmann::json::parse(bytes.cbegin(), bytes.cend());
            if (!json.is_object())
            {
                return Result::failure(ErrorCode::ParseFailed, QStringLiteral("配置文档根节点必须是 JSON 对象。"), m_filePath);
            }
            const int sourceVersion = json.value("_schema_version", 0);
            if (sourceVersion < 0)
            {
                return Result::failure(ErrorCode::UnsupportedVersion, QStringLiteral("配置版本不能为负数。"), m_filePath);
            }
            if (sourceVersion > T::SchemaVersion)
            {
                return Result::failure(
                    ErrorCode::UnsupportedVersion, QStringLiteral("配置版本 %1 高于当前支持的版本 %2。").arg(sourceVersion).arg(T::SchemaVersion), m_filePath);
            }
            if (sourceVersion < T::SchemaVersion)
            {
                Result migration = T::migrate(json, sourceVersion, T::SchemaVersion);
                if (!migration)
                {
                    migration.errorCode = ErrorCode::MigrationFailed;
                    migration.filePath = m_filePath;
                    return migration;
                }
            }

            T candidate = json.get<T>();
            T::normalize(candidate);
            const QList<Issue> issues = T::validate(candidate);
            if (!issues.isEmpty())
            {
                return Result::validationFailure(issues, m_filePath);
            }

            const nlohmann::json candidateJson = candidate;
            bool requiresRewrite = sourceVersion < T::SchemaVersion;
            nlohmann::json repairedJson = json;
            for (const auto &[key, value] : candidateJson.items())
            {
                const auto current = json.find(key);
                if (current == json.end() || *current != value)
                {
                    requiresRewrite = true;
                    repairedJson[key] = value;
                }
            }
            if (requiresRewrite)
            {
                const Result saveResult = saveCandidateLocked(std::move(repairedJson));
                if (!saveResult)
                {
                    return saveResult;
                }
            }
            return publishLoaded(std::move(candidate), firstLoad);
        } catch (const std::exception &exception)
        {
            return Result::failure(ErrorCode::ParseFailed, QStringLiteral("解析配置文件失败：%1").arg(QString::fromUtf8(exception.what())), m_filePath);
        }
    }

    /**
     * @brief 在持有写锁时校验、保存并发布候选配置。
     * @param[in] candidate 候选配置。
     * @param[in] forceSave 配置值未变化时是否仍写入文件。
     * @return 更新结果。
     */
    [[nodiscard]] Result setLocked(T candidate, bool forceSave)
    {
        if (!m_initialized)
        {
            return Result::failure(ErrorCode::NotInitialized, QStringLiteral("配置存储尚未初始化。"), m_filePath);
        }

        T::normalize(candidate);
        const QList<Issue> issues = T::validate(candidate);
        if (!issues.isEmpty())
        {
            return Result::validationFailure(issues, m_filePath);
        }

        try
        {
            const nlohmann::json currentJson = m_config;
            const nlohmann::json candidateJson = candidate;
            const bool changed = currentJson != candidateJson;
            if (!changed && !forceSave)
            {
                Result result;
                result.filePath = m_filePath;
                result.revision = m_revision;
                return result;
            }

            Result saveResult = saveCandidateLocked(candidateJson);
            if (!saveResult)
            {
                return saveResult;
            }
            if (changed)
            {
                m_config = std::move(candidate);
                ++m_revision;
            }
            saveResult.changed = changed;
            saveResult.revision = m_revision;
            return saveResult;
        } catch (const std::exception &exception)
        {
            return Result::failure(ErrorCode::SerializationFailed, QStringLiteral("序列化配置失败：%1").arg(QString::fromUtf8(exception.what())), m_filePath);
        }
    }

    /**
     * @brief 在持有写锁时将已序列化的候选配置原子写入文件。
     * @param[in] candidateJson 候选配置 JSON。
     * @return 文件保存结果。
     */
    [[nodiscard]] Result saveCandidateLocked(nlohmann::json candidateJson) const
    {
        candidateJson["_schema_version"] = T::SchemaVersion;
        QByteArray bytes = QByteArray::fromStdString(candidateJson.dump(4));
        bytes.append('\n');

        const QString directory = QFileInfo(m_filePath).absolutePath();
        if (!QDir().mkpath(directory))
        {
            return Result::failure(ErrorCode::InvalidPath, QStringLiteral("创建配置目录“%1”失败。").arg(directory), m_filePath);
        }

        QSaveFile file(m_filePath);
        file.setDirectWriteFallback(false);
        if (!file.open(QIODevice::WriteOnly))
        {
            return Result::failure(ErrorCode::OpenFailed, QStringLiteral("打开配置文件失败：%1").arg(file.errorString()), m_filePath);
        }
        if (file.write(bytes) != bytes.size())
        {
            file.cancelWriting();
            return Result::failure(ErrorCode::WriteFailed, QStringLiteral("写入配置文件失败：%1").arg(file.errorString()), m_filePath);
        }
        if (!file.commit())
        {
            return Result::failure(ErrorCode::CommitFailed, QStringLiteral("提交配置文件失败：%1").arg(file.errorString()), m_filePath);
        }

        Result result;
        result.filePath = m_filePath;
        return result;
    }

    /**
     * @brief 在持有写锁时发布成功加载的配置。
     * @param[in] candidate 已完成规范化和校验的配置。
     * @param[in] firstLoad 是否为首次加载。
     * @return 发布结果。
     */
    [[nodiscard]] Result publishLoaded(T candidate, bool firstLoad)
    {
        try
        {
            const bool changed = firstLoad || nlohmann::json(m_config) != nlohmann::json(candidate);
            if (changed)
            {
                m_config = std::move(candidate);
                ++m_revision;
            }
            Result result;
            result.filePath = m_filePath;
            result.changed = changed;
            result.revision = m_revision;
            return result;
        } catch (const std::exception &exception)
        {
            return Result::failure(ErrorCode::SerializationFailed, QStringLiteral("比较配置失败：%1").arg(QString::fromUtf8(exception.what())), m_filePath);
        }
    }

    mutable QReadWriteLock m_lock;
    QString m_filePath;
    T m_config{};
    quint64 m_revision = 0;
    bool m_initialized = false;
};

} // namespace Config

#endif // CONFIGSTORE_H
