#include "qsettingsidentitystore.h"

#include "path.h"

#include <QDir>
#include <QSettings>
#include <QSysInfo>
#include <QUuid>

#include <utility>

namespace
{
void setError(QString *errorMessage, const QString &message)
{
    if (errorMessage)
    {
        *errorMessage = message;
    }
}

QString identityConfigPath()
{
    return Utils::Path::configFile(QStringLiteral("identity.ini"));
}

bool ensureConfigDirectory(QString *errorMessage)
{
    if (QDir().mkpath(Utils::Path::configDirectory()))
    {
        return true;
    }
    setError(errorMessage, QStringLiteral("无法创建身份配置目录。"));
    return false;
}

} // namespace

bool QSettingsIdentityStore::load(Network::LocalIdentity *identity,
                                  QString *errorMessage)
{
    if (!identity)
    {
        setError(errorMessage, QStringLiteral("身份信息输出参数无效。"));
        return false;
    }

    if (!ensureConfigDirectory(errorMessage))
    {
        return false;
    }

    QSettings settings(identityConfigPath(), QSettings::IniFormat);
    Network::LocalIdentity loaded;
    loaded.deviceId = settings.value(QStringLiteral("network/deviceId"))
                          .toString()
                          .trimmed();
    if (loaded.deviceId.isEmpty())
    {
        loaded.deviceId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        settings.setValue(QStringLiteral("network/deviceId"), loaded.deviceId);
    }

    loaded.displayName = settings.value(QStringLiteral("profile/displayName"))
                             .toString()
                             .trimmed();
    if (loaded.displayName.isEmpty())
    {
        loaded.displayName = QSysInfo::machineHostName().trimmed();
    }
    if (loaded.displayName.isEmpty())
    {
        loaded.displayName = QStringLiteral("YueLink 用户");
    }
    loaded.displayName = loaded.displayName.left(64);

    settings.sync();
    if (settings.status() != QSettings::NoError)
    {
        setError(errorMessage, QStringLiteral("无法读取或初始化身份信息。"));
        return false;
    }

    *identity = std::move(loaded);
    setError(errorMessage, {});
    return true;
}

bool QSettingsIdentityStore::save(const Network::LocalIdentity &identity,
                                  QString *errorMessage)
{
    if (identity.deviceId.isEmpty() || identity.displayName.isEmpty())
    {
        setError(errorMessage, QStringLiteral("身份信息无效。"));
        return false;
    }

    if (!ensureConfigDirectory(errorMessage))
    {
        return false;
    }

    QSettings settings(identityConfigPath(), QSettings::IniFormat);
    settings.setValue(QStringLiteral("network/deviceId"), identity.deviceId);
    settings.setValue(QStringLiteral("profile/displayName"), identity.displayName);
    settings.sync();
    if (settings.status() != QSettings::NoError)
    {
        setError(errorMessage, QStringLiteral("无法保存身份信息。"));
        return false;
    }

    setError(errorMessage, {});
    return true;
}
