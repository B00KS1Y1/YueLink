#include "configstore.h"

#include "configserializer.h"
#include "infrastructure/path.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>

#include <exception>
#include <utility>

namespace Config
{

template <typename T>
ConfigStore<T>::ConfigStore(QString fileName, T defaults)
: m_fileName(std::move(fileName))
, m_defaults(std::move(defaults))
, m_config(m_defaults)
{
}

template <typename T> T ConfigStore<T>::get() const
{
    return m_config;
}

template <typename T> void ConfigStore<T>::set(T config)
{
    m_config = std::move(config);
}

template <typename T> Result ConfigStore<T>::load()
{
    const QString path = Utils::Path::configFile(m_fileName);
    const QFileInfo info(path);
    if (!info.exists())
    {
        m_config = m_defaults;
        return {};
    }
    if (!info.isFile())
    {
        return Result::failure(QStringLiteral("配置路径不是常规文件。"));
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        return Result::failure(QStringLiteral("打开配置文件失败：%1").arg(file.errorString()));
    }

    const QByteArray bytes = file.readAll();
    if (file.error() != QFileDevice::NoError)
    {
        return Result::failure(QStringLiteral("读取配置文件失败：%1").arg(file.errorString()));
    }

    try
    {
        const nlohmann::json json = nlohmann::json::parse(bytes.cbegin(), bytes.cend());
        T config = json.get<T>();
        m_config = std::move(config);
        return {};
    } catch (const std::exception &exception)
    {
        return Result::failure(QStringLiteral("解析配置文件失败：%1").arg(QString::fromUtf8(exception.what())));
    }
}

template <typename T> Result ConfigStore<T>::save() const
{
    QByteArray bytes;
    try
    {
        const nlohmann::json json = m_config;
        bytes = QByteArray::fromStdString(json.dump(4));
        bytes.append('\n');
    } catch (const std::exception &exception)
    {
        return Result::failure(QStringLiteral("序列化配置失败：%1").arg(QString::fromUtf8(exception.what())));
    }

    const QString path = Utils::Path::configFile(m_fileName);
    const QString directory = QFileInfo(path).absolutePath();
    if (!QDir().mkpath(directory))
    {
        return Result::failure(QStringLiteral("创建配置目录“%1”失败。").arg(directory));
    }

    QSaveFile file(path);
    file.setDirectWriteFallback(false);
    if (!file.open(QIODevice::WriteOnly))
    {
        return Result::failure(QStringLiteral("打开配置文件失败：%1").arg(file.errorString()));
    }
    if (file.write(bytes) != bytes.size())
    {
        file.cancelWriting();
        return Result::failure(QStringLiteral("写入配置文件失败：%1").arg(file.errorString()));
    }
    if (!file.commit())
    {
        return Result::failure(QStringLiteral("提交配置文件失败：%1").arg(file.errorString()));
    }
    return {};
}

template class ConfigStore<LogConfig>;
template class ConfigStore<ThemeConfig>;
template class ConfigStore<DatabaseConfig>;
template class ConfigStore<ApplicationConfig>;

ConfigStore<LogConfig> log{QStringLiteral("log.json")};
ConfigStore<ThemeConfig> theme{QStringLiteral("theme.json")};
ConfigStore<DatabaseConfig> database{QStringLiteral("database.json")};
ConfigStore<ApplicationConfig> application{QStringLiteral("application.json")};

} // namespace Config
